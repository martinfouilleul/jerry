//
//      .-. .-.   .-. .-. . . .-. .-. .-. .
//      |(   |    |   | | |\|  |  |(  | | |
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-'
//
//      ActiveControlFilter.cpp
//      Thibault Geoffroy
//

#include "ActiveControlFilter.h"
#include "SignalConverter.h"
#include <cstdlib>
#include <math.h>
#include <vector>
#include <iostream>

using namespace std;

#define Square(a) ((a)*(a))

const float RO_AIR = 1.225,
	    C0_AIR = 334,
	    TOM_RADIUS = 0.165,
	    TOM_HEIGHT = 0.30,
	    TOM_SURFACE = M_PI*Square(TOM_RADIUS),
	    TOM_VOLUME = TOM_HEIGHT*TOM_SURFACE;


const float ACF_SAMPLE_RATE		= 44100,
	    ACF_INIT_OMEGA		= 2*M_PI*200,
	    ACF_INIT_OMEGA_DESIRED	= ACF_INIT_OMEGA,
	    ACF_INIT_ZETA		= 15.,		// Measured
	    ACF_INIT_ZETA_DESIRED	= ACF_INIT_ZETA,
	    ACF_INIT_GAMMA		= RO_AIR * Square(C0_AIR) * Square(TOM_SURFACE) / TOM_VOLUME,
	    ACF_INIT_L1			= .1,
	    ACF_INIT_L2			= .1;

//TODO(martin): pass filter coeffs as parameters to the ActiveControlFilter's constructor ?

const float ACF_SPEAKER_COEFF0		= -2.069471,
	    ACF_SPEAKER_COEFF1		= 1.773126,
	    ACF_SPEAKER_COEFF2		= -0.05492778;

ActiveControlFilter::ActiveControlFilter( ControlLoopFacade& loop ): FilterInterface( loop ),
								     controller(ACF_SAMPLE_RATE,
										ACF_INIT_OMEGA,
										ACF_INIT_ZETA,
										ACF_INIT_GAMMA,
										ACF_INIT_OMEGA_DESIRED,
										ACF_INIT_ZETA_DESIRED,
										ACF_INIT_L1,
										ACF_INIT_L2)

{
	filterCoeffs[0] = ACF_SPEAKER_COEFF0;
	filterCoeffs[1] = ACF_SPEAKER_COEFF1;
	filterCoeffs[2] = ACF_SPEAKER_COEFF2;

	filterSamples[0] = 0;
	filterSamples[1] = 0;

	enable(true);
}

ActiveControlFilter::~ActiveControlFilter(){}
bool ActiveControlFilter::initialize(){}
void ActiveControlFilter::resetForStartup(){ initialize();}

float ActiveControlFilter::step( float input, long long t )
{
	//NOTE(martin): get the next control command
	float command = controller.nextStep(input);

	//NOTE(martin): apply the inverse Speaker response before sending it to the output
	float output = filterCoeffs[0] * command + filterCoeffs[1] * filterSamples[0] + filterCoeffs[2] * filterSamples[1] ;
	filterSamples[1] = filterSamples[0];
	filterSamples[0] = command;

	return(output);
}

bool ActiveControlFilter::handleMessage( const string& message )
{
    size_t pos = message.find( "/coala/set/activecontrol/enable/" );
    if ( pos == 0 )
    {
        string value = message.substr( 25 );
        enable( (bool ) atoi( value.c_str()) );
        return true;
    }
    pos = message.find( "/?activecontrolon=" );
    if ( pos == 0 )
    {
        string value = message.substr( 11 );
        enable( (bool ) atoi( value.c_str()) );
        return true;
    }

    pos = message.find("/coala/set/activecontrol/omegad=");
    if(pos == 0)
    {
	string value = message.substr(32);
	controller.m_omegaDesired = (float)atof(value.c_str());
	return(true);
    }

    return false;
}

void ActiveControlFilter::serializeStatus( std::stringstream& stream ) const
{}
