//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      (C) Ircam 2015
//      ChirpFilter.h
//      programmed by Robert Piechaud
//

#include <stdio.h>
#include "core/FilterInterface.h"

#ifndef _CHIRP_FILTER_H_
#define _CHIRP_FILTER_H_

class RealTimeInterface;
class ControlLoopFacade;

class ChirpFilter: public FilterInterface
{
public:
    ChirpFilter( ControlLoopFacade& loop );
    virtual ~ChirpFilter();
    
    virtual bool         initialize();
    virtual void         resetForStartup();
    virtual float        step( float input, long long t );
    virtual bool         handleMessage( const std::string& message );
    virtual void         serializeStatus( std::stringstream& stream ) const;
    virtual bool         hasSelfExcitation() const { return true; }
    
    void  setParameters( float f0, float f1 );

private:
    float chirpGain_;
    float f0_;
    float f1_;
};

#endif // _CHIRP_FILTER_H_
