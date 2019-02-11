//
//      .-. .-.   .-. .-. . . .-. .-. .-. .
//      |(   |    |   | | |\|  |  |(  | | |
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-'
//
//      ActiveControlFilter.cpp
//      Martin Fouilleul & Thibault Geoffroy
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
	    ACF_INIT_OMEGA		= 2*M_PI*218,
	    ACF_INIT_OMEGA_DESIRED	= 2*M_PI*190,
	    ACF_INIT_ZETA		= 15. / ACF_INIT_OMEGA,		// Measured
	    ACF_INIT_ZETA_DESIRED	= ACF_INIT_ZETA,
	    ACF_INIT_GAMMA		= RO_AIR * Square(C0_AIR) * Square(TOM_SURFACE) / TOM_VOLUME,
	    ACF_INIT_L1			= .1,
	    ACF_INIT_L2			= .1;


const float ACF_FS_TO_PA		= 3.3 / 0.00316 ;

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
										ACF_INIT_L2),
								     speakerFilterSamples(),
								     micFilterSamples()


{
	speakerFilterCoeffs[0] = ACF_SPEAKER_COEFF0;
	speakerFilterCoeffs[1] = ACF_SPEAKER_COEFF1;
	speakerFilterCoeffs[2] = ACF_SPEAKER_COEFF2;

	//NOTE(martin): Biquad coefficients for a lowpass filter with Fc = 500Hz Q = 0.707
	//		in the order a1, a2, b0, b1, b2
	//		See biquad calculator at https://arachnoid.com/BiQuadDesigner/

	micFilterCoeffs[0] = -1.89931967;
	micFilterCoeffs[1] = 0.90414926;
	micFilterCoeffs[2] = 0.00120740;
	micFilterCoeffs[3] = 0.00241479;
	micFilterCoeffs[4] = 0.00120740;

	enable(true);
}

ActiveControlFilter::~ActiveControlFilter(){}
bool ActiveControlFilter::initialize(){}
void ActiveControlFilter::resetForStartup(){ initialize();}

float ActiveControlFilter::step( float input, long long t )
{
	//NOTE(martin): Filter the microphone input with a low pass to eliminate higher modes

	input *= ACF_FS_TO_PA ;
	input -= speakerFilterSamples[0];

	float filteredInput = micFilterCoeffs[2] * input + micFilterCoeffs[3] * micFilterSamples[2] + micFilterCoeffs[4] * micFilterSamples[3]
			     - micFilterCoeffs[0] * micFilterSamples[0] - micFilterCoeffs[1] * micFilterSamples[1] ;

	micFilterSamples[1] = micFilterSamples[0];
	micFilterSamples[0] = filteredInput;

	micFilterSamples[3] = micFilterSamples[2];
	micFilterSamples[2] = input;

	//NOTE(martin): get the next control command
	float command = controller.nextStep(filteredInput);

	//NOTE(martin): apply the inverse Speaker response before sending it to the output
	float output = speakerFilterCoeffs[0] * command + speakerFilterCoeffs[1] * speakerFilterSamples[0] + speakerFilterCoeffs[2] * speakerFilterSamples[1] ;

	speakerFilterSamples[1] = speakerFilterSamples[0];
	speakerFilterSamples[0] = command;

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
