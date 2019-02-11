//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      (C) Ircam 2015
//      ModulatedGainFilter.h
//      programmed by Robert Piechaud
//

#include <stdio.h>
#include "core/FilterInterface.h"

#ifndef _MODULATED_GAIN_FILTER_H_
#define _MODULATED_GAIN_FILTER_H_

class RealTimeInterface;
class ControlLoopFacade;

class ModulatedGainFilter: public FilterInterface
{
public:
    ModulatedGainFilter( ControlLoopFacade& loop );
    virtual ~ModulatedGainFilter();
    
    virtual bool         initialize();
    virtual void         resetForStartup();
    virtual float        step( float input, long long t );
    virtual bool         handleMessage( const std::string& message );
    virtual void         serializeStatus( std::stringstream& stream ) const;
    
    void  setParameter( float freq );

private:
    float freq_;
    float freqOld_;
    float a1_;
};

#endif // _MODULATED_GAIN_FILTER_H_
