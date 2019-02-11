//
//      .-. .-.   .-. .-. . . .-. .-. .-. .
//      |(   |    |   | | |\|  |  |(  | | |
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-'
//
//      ActiveControlFilter.h
//      Martin Fouilleul & Thibault Geoffroy
//

#pragma once

#include <stdio.h>
#include "TomControl.h"
#include "core/FilterInterface.h"


class RealTimeInterface;
class ControlLoopFacade;

class ActiveControlFilter: public FilterInterface
{
public:
    ActiveControlFilter( ControlLoopFacade& loop );
    virtual ~ActiveControlFilter();

    virtual bool         initialize();
    virtual void         resetForStartup();
    virtual float        step( float input, long long t );
    virtual bool         handleMessage( const std::string& message );
    virtual void         serializeStatus( std::stringstream& stream ) const;

    TomController	controller;
    float		speakerFilterCoeffs[3];
    float		speakerFilterSamples[2];

    float		micFilterCoeffs[5];	// a1, a2, b0, b1, b2
    float		micFilterSamples[4];	// y[n-1], y[n-2], x[n-1], x[n-2]

};
