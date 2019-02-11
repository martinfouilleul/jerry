//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      © Ircam 2014
//      SignalConverter.h
//      programmed by Robert Piechaud
//
//      This class reflect the use of TI's DAC DAC8551 & ADC ADS8860 (16bit converters)
//		interfaced via SPI with the BeagleBone Black PRU and SmartInstruments 0.1 cape.
//

#include <stdio.h>
#include "core/SignalConverterInterface.h"

#ifndef _SIGNAL_CONVERTER_H_
#define _SIGNAL_CONVERTER_H_


#define VOLT_MAX_ABS  3.3
#define SAMPLE_MAX    65535

class RealTimeInterface;
class PrussManager;

class SignalConverter: public SignalConverterInterface
{
public:
    SignalConverter( RealTimeInterface& realtime, PrussManager& pruss );
    virtual ~SignalConverter();

    virtual bool  initialize();
    virtual void  outputOneSampleNow( unsigned int channel, float value );
    virtual float acquireOneSampleNow( unsigned int channel );
    virtual void  test();
    virtual float getMinValue() const { return 0; };
    virtual float getMaxValue() const { return VOLT_MAX_ABS; };

private:
    float testInputSynchronousMinNanosecondTime();
    float testOutputSynchronousMinNanosecondTime();

private:
    RealTimeInterface&  realtime_;
    PrussManager&       pruss_;
    float               voltMax_;
    unsigned short      lastInputValue_;
    unsigned short      lastOutputValue_;
};

#endif // _SIGNAL_CONVERTER_H_
