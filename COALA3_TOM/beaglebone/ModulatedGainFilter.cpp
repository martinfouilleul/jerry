//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      (C) Ircam 2015
//      ModulatedGainFilter.cpp
//      programmed by Robert Piechaud
//

#include "ModulatedGainFilter.h"
#include <cstdlib>
#include <math.h>
#include <vector>

using namespace std;

ModulatedGainFilter::ModulatedGainFilter( ControlLoopFacade& loop ):
    FilterInterface( loop ),
    freq_    (0.),
    freqOld_ (0.),
    a1_      (0.)
{
    //NOTHING
}

ModulatedGainFilter::~ModulatedGainFilter()
{
    //NOTHING
}

void ModulatedGainFilter::setParameter( float freq )
{
    freq_ = freq;
}

bool ModulatedGainFilter::initialize()
{
    //NOTHING
}

void ModulatedGainFilter::resetForStartup()
{
    initialize();
}

float ModulatedGainFilter::step( float input, long long t ) // t in nanosec.
{
    float output = input;
    long long t0 = loop_.getStartTime();
    double time = (double)(t - t0)*1.0e-9;
    float f = freq_;
    if ( freqOld_ != freq_ && freq_ != 0. )
    {
        a1_ = time+(freqOld_/freq_)*(a1_ - time);
        freqOld_ = freq_;
    }
    output *= cos(2*M_PI*f*(time - a1_));
    return output;
}

bool ModulatedGainFilter::handleMessage( const string& message )
{
    size_t pos = message.find( "/coala/set/modulatedgain/enable/" );
    if ( pos == 0 )
    {
        string value = message.substr( 32 );
        enable( (bool ) atoi( value.c_str()) );
        return true;
    }
    pos = message.find( "/?modulatedgainon=" );
    if ( pos == 0 )
    {
        string value = message.substr( 18 );
        enable( (bool ) atoi( value.c_str()) );
        return true;
    }
    // we expect: /coala/set/modulatedgainfreq/{float value}
    pos = message.find( "/coala/set/modulatedgainfreq/" );
    if ( pos == 0 )
    {
        string value = message.substr( 29 );
        freq_ = atof( value.c_str());
        return true;
    }
    pos = message.find( "/coala/set/control/param1/" );  //legacy param name...
    if ( pos == 0 )
    {
        string value = message.substr( 26 );
        freq_ = atof( value.c_str());
        return true;
    }
    return false;
}

void ModulatedGainFilter::serializeStatus( std::stringstream& stream ) const
{
    stream << "modulated_gain=" << (enabled()?"true":"false") << "+modulated_gain_freq=" << freq_ << "+";
}
