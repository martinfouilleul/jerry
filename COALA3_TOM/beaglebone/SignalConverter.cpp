//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      © Ircam 2014
//      SignalConverter.ccp
//      programmed by Robert Piechaud
//
//      This class reflect the use of TI's DAC DAC8551 & ADC ADS8860 (16bit converters)
//		interfaced via SPI with the BeagleBone Black PRU and SmartInstruments 0.1 cape.
//

#include "SignalConverter.h"
#include "PrussManager.h"
#include "core/RealTimeInterface.h"
#include <stdlib.h>
#include <unistd.h>

SignalConverter::SignalConverter( RealTimeInterface& realtime, PrussManager& pruss ) :
    SignalConverterInterface (),
    realtime_ ( realtime ),
    pruss_    ( pruss ),
    lastInputValue_ ( 0 ),
    lastOutputValue_( 0 )
{
    //NOTHING
}

SignalConverter::~SignalConverter()
{
    //NOTHING
}

bool SignalConverter::initialize()
{
    ((unsigned int*)pruss_.getPruMemory( 1 ))[OFFSET_DATAIN_REQUEST/sizeof(unsigned int)] = 0;
    printf("SignalConverter: initialisation\n");
    return true;
}

void SignalConverter::outputOneSampleNow( unsigned int /* channel */, float value )
{
    long count = 0;
    long long t = realtime_.getHighPrecisionNanosecondTime();
    //printf("DAC input value = %g at %lld\n", value, t);
    while ( ((unsigned int*)pruss_.getPruMemory( 1 ))[OFFSET_DATAOUT_DONE/sizeof(unsigned int)] == 1 ){}
    // -1.65 <= value <= +1.65 => 0 <= value <= +3.3:
    value += VOLT_MAX_ABS/2;
    // 0 <= value <= +3.3 => 0 <= raw <= 65535:
    int rawValue = (int)((float)value*SAMPLE_MAX/VOLT_MAX_ABS);
    if ( rawValue < 0 )
      rawValue = 0;
    else if ( rawValue > SAMPLE_MAX )
      rawValue = SAMPLE_MAX;  // cut at SAMPLE_MAX
    // ((unsigned int*)pruss_.getPruMemory( 1 ))[OFFSET_DATAOUT/4] = (unsigned int) ((float)value*SAMPLE_MAX/VOLT_MAX_ABS);
    ((unsigned int*)pruss_.getPruMemory( 1 ))[OFFSET_DATAOUT/sizeof(unsigned int)] = (unsigned int) rawValue;
    ((unsigned int*)pruss_.getPruMemory( 1 ))[OFFSET_DATAOUT_READY/sizeof(unsigned int)] = 1;
    count = 0;
    while ( ((unsigned int*)pruss_.getPruMemory( 1 ))[OFFSET_DATAOUT_DONE/sizeof(unsigned int)] == 0 ){}
    ((unsigned int*)pruss_.getPruMemory( 1 ))[OFFSET_DATAOUT_READY/sizeof(unsigned int)] = 0;
    t = realtime_.getHighPrecisionNanosecondTime();
    //printf("DAC output at %lld\n", t);
}

float SignalConverter::acquireOneSampleNow( unsigned int /* channel */ )
{
    //printf("ADC input DATA_IN_READY = %u \n", ((unsigned int*)pruss_.getPruMemory( 1 ))[OFFSET_DATAIN_READY/sizeof(unsigned int)] );
    long long t1= realtime_.getHighPrecisionNanosecondTime();
    ((unsigned int*)pruss_.getPruMemory( 1 ))[OFFSET_DATAIN_REQUEST/sizeof(unsigned int)] = 1;
    long count = 0;
    while ( ((unsigned int*)pruss_.getPruMemory( 1 ))[OFFSET_DATAIN_READY/sizeof(unsigned int)] == 0 ){}
    long long t2= realtime_.getHighPrecisionNanosecondTime();
    ((unsigned int*)pruss_.getPruMemory( 1 ))[OFFSET_DATAIN_REQUEST/sizeof(unsigned int)] = 0;
    // analog input value is [-1.65... +1.65] and then with added continuous component => v is in [0...+3.3],
    // then centering around 0: 0 <= v < 3.3V => -32768 < raw < 32678:
    short rawValue = (short)((unsigned int*)pruss_.getPruMemory( 1 ))[OFFSET_DATAIN/sizeof(unsigned int)] - (SAMPLE_MAX + 1)/2;
    //printf( "ADC: %x\n", raw ); 
   // phase inversion and scaling [-1.65... +1.65]
    float value = -(float) rawValue*VOLT_MAX_ABS/SAMPLE_MAX; // rephasing
    long long t3= realtime_.getHighPrecisionNanosecondTime();
    //printf("ADC %g %lld %lld %lld \n",value, t1, t2-t1, t3-t1);
    return value;
}

void SignalConverter::test()
{
    testInputSynchronousMinNanosecondTime();
    testOutputSynchronousMinNanosecondTime();
}

float SignalConverter::testInputSynchronousMinNanosecondTime()
{
    const int steps = 10000;
    long long delta [steps];
    long long t0, t1;
    const int warmUp = 20;

    printf( "Signal Converter : evaluating typical acquisition time over %d trials.\n", steps );
    for ( int n = 0; n < steps; ++n )
    {
        if ( n < warmUp )
            continue;
        t0 = realtime_.getHighPrecisionNanosecondTime();
        float input = this->acquireOneSampleNow( 0 );
        t1 = realtime_.getHighPrecisionNanosecondTime();
        delta[n] = t1-t0;
    }
    long long max = 0;
    double sum = 0;
    long maxIndex = 0;
    const long nsThreshold = 100000;
    int maxNum = 0;
    int effectiveSteps = 0;
    for ( int n = warmUp + 1; n < steps; ++n )
    {
        if ( delta[n] > max )
        {
            max = delta[n];
            maxIndex = n;
        }
        if ( delta[n] > nsThreshold )
            ++maxNum;
        else
        {
            sum += delta[n];
            ++effectiveSteps;
        }
    }
    float mean = sum/effectiveSteps;
    float nsTime = (float) mean*1.25*1.0e-9;
    float frequency = 1/nsTime;
    sum *= 1.0e-3;
    printf( "\t=> mean time: %.2f µs, max time: %.2f µs (n = %d), time > %d µs: %d, secure time: %.2f µs (freq: %.2f kHz)\n",
            mean*1.0e-3, (float)max*1.0e-3, maxIndex,
            nsThreshold/1000, maxNum, nsTime*1.0e6, frequency*1.0e-3 );
    return nsTime;
}

float SignalConverter::testOutputSynchronousMinNanosecondTime()
{
    const int steps = 10000;
    long long delta [steps];
    long long t0, t1;
    const int warmUp = 20;

    printf( "Signal Converter : evaluating typical output time over %d trials.\n", steps );
    for ( int n = 0; n < steps; ++n )
    {
        if ( n < warmUp )
            continue;
        t0 = realtime_.getHighPrecisionNanosecondTime();
        this->outputOneSampleNow( 0, (float) VOLT_MAX_ABS*n/SAMPLE_MAX );
        t1 = realtime_.getHighPrecisionNanosecondTime();
        delta[n] = t1-t0;
    }
    long long max = 0;
    double sum = 0;
    long maxIndex = 0;
    const long nsThreshold = 100000;
    int maxNum = 0;
    int effectiveSteps = 0;
    for ( int n = warmUp + 1; n < steps; ++n )
    {
        if ( delta[n] > max )
        {
            max = delta[n];
            maxIndex = n;
        }
        if ( delta[n] > nsThreshold )
            ++maxNum;
        else
        {
            sum += delta[n];
            ++effectiveSteps;
        }
    }
    float mean = sum/effectiveSteps;
    float nsTime = (float) mean*1.25*1.0e-9;
    float frequency = 1/nsTime;
    sum *= 1.0e-3;
    printf( "\t=> mean time: %.2f µs, max time: %.2f µs (n = %d), time > %d µs: %d, secure time: %.2f µs (freq: %.2f kHz)\n",
            mean*1.0e-3, (float)max*1.0e-3, maxIndex,
            nsThreshold/1000, maxNum, nsTime*1.0e6, frequency*1.0e-3 );
    return nsTime;
}

/*
void SignalConverter::sendSinus()
{   
    long long t = 0;
    long long t_max = 0;
    double sum = 0;
    int maxNum = 0;
    long long T = 22000;   //in nanoseconds
    float F = 500;
    long long t0 = realtime_.getHighPrecisionNanosecondTime();
    long long t_before = t0;
    long long nextTheoreticalTime = 0;
    long long currentTime = 0;
    float timeLimit = 30;
    long k = 0;
    while ( true )
    {
        nextTheoreticalTime = t0 + (k + 1)*T;
        while ( true )
        {
            t = realtime_.getHighPrecisionNanosecondTime();
            if ( t >= nextTheoreticalTime )
                break;
        }
        float value = (float)(VOLT_MAX_ABS/2)*(1+sin(2*PI*F*k*T*1.0e-9));
        outputOneSampleNow( 0, value );
        ++k;
        if ( (double)(t - t0)*1.0e-9 >= timeLimit )
            break;
    }
}
*/
