//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      (C) Ircam 2015
//      ChirpFilter.cpp
//      programmed by Robert Piechaud
//

#include "ChirpFilter.h"
#include "SignalConverter.h"
#include <cstdlib>
#include <math.h>
#include <vector>

using namespace std;

ChirpFilter::ChirpFilter( ControlLoopFacade& loop ):
    FilterInterface  ( loop ),
    chirpGain_       ( 0.3 ),
    f0_              ( 80 ),
    f1_              ( 2000 )
{
    //NOTHING
}

ChirpFilter::~ChirpFilter()
{
    //NOTHING
}

void ChirpFilter::setParameters( float f0, float f1 )
{
    f0_ = f0;
    f1_ = f1;
}

bool ChirpFilter::initialize()
{
    //NOTHING
}

void ChirpFilter::resetForStartup()
{
    initialize();
}

float ChirpFilter::step( float input, long long t ) // t in nanosec.
{
    long long t0 = loop_.getStartTime();
    
    float Ts = loop_.getSamplingTime(); //in seconds
    double time = (double)(t - t0)*1.0e-9;
    float f = (float) ( f0_ + (double)(t-t0)*1.0e-9*(f1_-f0_)/(2*loop_.getTimeLimit()) );
    float output = (float) chirpGain_* (VOLT_MAX_ABS/2) * sin(2*M_PI*f*time);
    /*
    if ( recordOn_ && k_ < recordLimit_ )
    {
        buffer4_[k_] = output;
    }
    */
    return output;
}

bool ChirpFilter::handleMessage( const string& message )
{
    size_t pos = message.find( "/coala/set/chirp/enable/" );
    if ( pos == 0 )
    {
        string value = message.substr( 24 );
        enable( (bool ) atoi( value.c_str()) );
        return true;
    }
    pos = message.find( "/coala/set/chirp/startfreq/" );
    if ( pos == 0 )
    {
        float value = atof( message.substr( 27 ).c_str());
        if ( value > 0. )
            f0_ = value;
        return true;
    }
    pos = message.find( "/coala/set/chirp/endfreq/" );
    if ( pos == 0 )
    {
        float value = atof( message.substr( 25 ).c_str());
        if ( value > 0. )
            f1_ = value;
        return true;
    }
    return false;
}

void ChirpFilter::serializeStatus( std::stringstream& stream ) const
{
    stream << "chirp=" << (enabled()?"true":"false") << "+start_freq=" << f0_ << "+end_freq=" << f1_ << "+";
}
