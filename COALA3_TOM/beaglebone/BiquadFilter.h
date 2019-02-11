//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      (C) Ircam 2015
//      BiquadFilter.h
//      programmed by Robert Piechaud
//

#include <stdio.h>
#include "core/FilterInterface.h"

#ifndef _BIQUAD_FILTER_H_
#define _BIQUAD_FILTER_H_

class RealTimeInterface;
class ControlLoopFacade;

enum BiquadFilterType
{
    Lowpass = 1,
    Highpass,
    Bandpass,
    Notch,
    Peak,
    Lowshelf,
    Highshelf
};

struct BiquadCoeffs
{
    double a0;
    double a1;
    double a2;
    double b1;
    double b2;
    
    void reset() { a0 = 0.; a1 = 0.; a2 = 0.; b1 = 0.; b2 = 0.; }
};

class BiquadFilter: public FilterInterface
{
public:
    BiquadFilter( ControlLoopFacade& loop );
    virtual ~BiquadFilter();
    
    virtual bool         initialize();
    virtual void         resetForStartup();
    virtual float        step( float input, long long t );
    virtual bool         handleMessage( const std::string& message );
    virtual void         serializeStatus( std::stringstream& stream ) const;
    virtual void         enable( bool status );
    
private:
    void        computeCoefficients();
    std::string typeToString() const;

private:
    BiquadFilterType type_;
    BiquadCoeffs     coeffs_;
    double           z1_, z2_;
    double           Fc_, Q_, peakGain_;
    double           factor_;
    long long        smoothStart_;
    bool             smoothing_;
};

#endif // _BIQUAD_FILTER_H_
