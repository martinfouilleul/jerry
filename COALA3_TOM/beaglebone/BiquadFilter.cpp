//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      (C) Ircam 2015
//      BiquadFilter.cpp
//      programmed by Robert Piechaud
//

#include "BiquadFilter.h"
#include <cstdlib>
#include <math.h>
#include <vector>
#include <iostream>

using namespace std;

#define SMOOTHING_NS 20000000

BiquadFilter::BiquadFilter( ControlLoopFacade& loop ):
    FilterInterface( loop ),
    type_ ( Lowpass ),
    z1_   (0.),
    z2_   (0.),
    Fc_   (500.),
    Q_    (0.7),
    peakGain_ (1.0),
    factor_ ( 1.0),
    smoothStart_ ( 0 ),
    smoothing_( false )
{
    coeffs_.reset();
}

BiquadFilter::~BiquadFilter()
{
    //NOTHING
}

bool BiquadFilter::initialize()
{
    z1_ = 0.;
    z2_ = 0.;
    computeCoefficients();
    smoothStart_ = 0;
    factor_ = 1.;
}

void BiquadFilter::resetForStartup()
{
    initialize();
}


string BiquadFilter::typeToString() const
{
    string text;
    switch (type_)
    {
        case Lowpass: text = "lowpass"; break;
        case Highpass: text = "highpass"; break;
        case Bandpass: text = "bandpass"; break;
        case Notch: text = "notch"; break;
        case Peak: text = "peak"; break;
        case Lowshelf: text = "lowshelf"; break;
        case Highshelf: text = "highshelf"; break;
        default: text = "unknown"; break;
    }
    return text;
}

void BiquadFilter::enable( bool status )
{
    if ( !loop_.isRunning() )
        return FilterInterface::enable( status );
    if ( enabled() == status || !enabled() && disabling() )
        return;
    if ( !enabled() )
        FilterInterface::enable( true );
    else
        setDisabling( true );
    smoothing_ = true;
    smoothStart_ = 0;
};

void BiquadFilter::computeCoefficients()
{
    double A, omega, sn, cs, alpha, beta;
    double a0, a1, a2, b0, b1, b2;
    
    cout << "sampling time: " << loop_.getSamplingTime();
    A = pow(10, peakGain_ /40);
    omega = 2 * M_PI * Fc_ * loop_.getSamplingTime();
    sn = sin(omega);
    cs = cos(omega);
    alpha = sn / (2*Q_); //sinh(M_LN2 /2 * bandwidth * omega /sn);
    beta = sqrt(A + A);
    
    switch ( type_ )
    {
        case Lowpass:
        {
            b0 = (1 - cs) /2;
            b1 = 1 - cs;
            b2 = (1 - cs) /2;
            a0 = 1 + alpha;
            a1 = -2 * cs;
            a2 = 1 - alpha;
            break;
        }
            
        case Highpass:
        {
            b0 = (1 + cs) /2;
            b1 = -(1 + cs);
            b2 = (1 + cs) /2;
            a0 = 1 + alpha;
            a1 = -2 * cs;
            a2 = 1 - alpha;
            break;
        }
            
        case Bandpass:
        {
            b0 = alpha;
            b1 = 0;
            b2 = -alpha;
            a0 = 1 + alpha;
            a1 = -2 * cs;
            a2 = 1 - alpha;
            break;
        }
            
        case Notch:
        {
            b0 = 1;
            b1 = -2 * cs;
            b2 = 1;
            a0 = 1 + alpha;
            a1 = -2 * cs;
            a2 = 1 - alpha;
            break;
        }
            
        case Peak:
        {
            b0 = 1 + (alpha * A);
            b1 = -2 * cs;
            b2 = 1 - (alpha * A);
            a0 = 1 + (alpha /A);
            a1 = -2 * cs;
            a2 = 1 - (alpha /A);
            break;
        }
            
        case Lowshelf:
        {
            b0 = A * ((A + 1) - (A - 1) * cs + beta * sn);
            b1 = 2 * A * ((A - 1) - (A + 1) * cs);
            b2 = A * ((A + 1) - (A - 1) * cs - beta * sn);
            a0 = (A + 1) + (A - 1) * cs + beta * sn;
            a1 = -2 * ((A - 1) + (A + 1) * cs);
            a2 = (A + 1) + (A - 1) * cs - beta * sn;
            break;
        }
            
        case Highshelf:
        {
            b0 = A * ((A + 1) + (A - 1) * cs + beta * sn);
            b1 = -2 * A * ((A - 1) + (A + 1) * cs);
            b2 = A * ((A + 1) + (A - 1) * cs - beta * sn);
            a0 = (A + 1) - (A - 1) * cs + beta * sn;
            a1 = 2 * ((A - 1) - (A + 1) * cs);
            a2 = (A + 1) - (A - 1) * cs - beta * sn;
            break;
        }
    }
    
    coeffs_.a0 = b0 /a0;
    coeffs_.a1 = b1 /a0;
    coeffs_.a2 = b2 /a0;
    coeffs_.b1 = a1 /a0;
    coeffs_.b2 = a2 /a0;
    
    //z1_ = z2_ = 0;
}

float BiquadFilter::step( float input, long long t )
{
    if ( smoothing_ )
    {
        if ( smoothStart_ == 0 )
            smoothStart_ = t;
        long long delta = t - smoothStart_;
        if ( delta > SMOOTHING_NS ) //finished
        {
            smoothStart_ = 0;
            smoothing_ = false;
            if ( disabling() )
            {
                setDisabling( false );
                FilterInterface::enable( false );
                factor_ = 0.;
            }
            else
                factor_ = 1.;
        }
        else
        {
            if ( disabling() )
                factor_ = 1. - ((double) delta/SMOOTHING_NS);
            else
                factor_ = (double) delta/SMOOTHING_NS;
        }
    }
    double output = input * coeffs_.a0 + z1_;
    z1_ = input * coeffs_.a1 + z2_ - coeffs_.b1 * output;
    z2_ = input * coeffs_.a2 - coeffs_.b2 * output;
    return (float) output * factor_;
}

bool BiquadFilter::handleMessage( const string& message )
{
    size_t pos = message.find( "/coala/set/biquad/enable/" );
    if ( pos == 0 )
    {
        string value = message.substr( 25 );
        enable( (bool ) atoi( value.c_str()) );
        return true;
    }
    pos = message.find( "/?biquadon=" );
    if ( pos == 0 )
    {
        string value = message.substr( 11 );
        enable( (bool ) atoi( value.c_str()) );
        return true;
    }
    // we expect: "/coala/set/biquad/coefficients/a0,a1,a2,b1,b2"
    /*
    pos = message.find( "/coala/set/biquad/coefficients/" );
    if ( pos == 0 )
    {
        string values = message.substr( 31 );
        vector<float> valueList;
        bool listOk = true;
        for ( int i = 0; i < values.length(); ++i)
        {
            if ( values[i] != ',' && values[i] != '-' && values[i] != '.' && !isdigit(values[i]))
                return false;
        }
        string sub;
        size_t pos = string::npos;
        do
        {
            pos = values.find(',');
            if ( pos == string::npos )
            {
                sub = values;
                values = "";
            }
            else
            {
                sub = values.substr( 0, pos );
                values = values.substr( pos + 1 );
            }
            valueList.push_back( atof( sub.c_str()));
        }
        while ( values.length() > 0 );
        if ( valueList.size() == 5 )
        {
            coeffs_.a0 = valueList[0];
            coeffs_.a1 = valueList[1];
            coeffs_.a2 = valueList[2];
            coeffs_.b1 = valueList[3];
            coeffs_.b2 = valueList[4];
            return true;
        }
    }
     */
    
    // we expect: "/coala/set/biquad/parameters/type|Fc|peakGain|Q"
    pos = message.find( "/coala/set/biquad/parameters/" );
    if ( pos == 0 )
    {
        string values = message.substr( 29 );
        vector<float> valueList;
        bool listOk = true;
        for ( int i = 0; i < values.length(); ++i)
        {
            if ( values[i] != '|' && values[i] != '-' && values[i] != '.' && !isdigit(values[i]))
                return false;
        }
        string sub;
        size_t pos = string::npos;
        do
        {
            pos = values.find('|');
            if ( pos == string::npos )
            {
                sub = values;
                values = "";
            }
            else
            {
                sub = values.substr( 0, pos );
                values = values.substr( pos + 1 );
            }
            valueList.push_back( atof( sub.c_str()));
        }
        while ( values.length() > 0 );
        if ( valueList.size() == 4 )
        {
            type_ = (BiquadFilterType) valueList[0];
            Fc_ = valueList[1];
            peakGain_ = valueList[2];
            Q_ = valueList[3];
            computeCoefficients();
            cout << "BiquadFilter: type: " << typeToString() << "Fc=" << Fc_ << " peak gain=" << peakGain_ << " Q=" << Q_ << endl;
            return true;
        }
    }
    return false;
}

void BiquadFilter::serializeStatus( std::stringstream& stream ) const
{
    stream << "biquad=" << (enabled()?"true":"false") << "+type=" << typeToString() << "_Fc=" << Fc_ << "_Q=" << Q_ << "_peak_gain=" << peakGain_ << "+";
    stream << "(biquad_coeffs=" << (double) coeffs_.a0 << "_" << (double) coeffs_.a1 << "_" << (double)coeffs_.a2 << "_" << (double)coeffs_.b1 << "_" << (double)coeffs_.b2 << ")+";
}
