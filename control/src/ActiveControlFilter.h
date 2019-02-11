//
//      .-. .-.   .-. .-. . . .-. .-. .-. .
//      |(   |    |   | | |\|  |  |(  | | |
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-'
//
//      ActiveControlFilter.h
//      Thibault Geoffroy
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
    float		filterCoeffs[3];
    float		filterSamples[2];
};
