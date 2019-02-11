//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      (C) Ircam 2015
//      GenFilter.h
//      programmed by Robert Piechaud
//

#include <stdio.h>
#include "core/FilterInterface.h"

#ifndef _GEN_FILTER_H_
#define _GEN_FILTER_H_

class ControlLoopFacade;

class GenFilter: public FilterInterface
{
public:
    GenFilter( ControlLoopFacade& loop );
    virtual ~GenFilter();
    
    virtual bool  initialize();
    virtual void  resetForStartup();
    virtual float step( float input, long long t );
    virtual bool  handleMessage( const std::string& message );
    virtual void  serializeStatus( std::stringstream& stream ) const;
    virtual bool  hasSelfExcitation() const { return true; }
    
    void recompileGenModule();

    bool loadGenLibrary();
    bool loadGenLibraryPlug(const char* name);
    void unloadGenLibrary();
    
private:
    bool instanciateLibFunction( const char* name, void** fun );
    void setParameter( const char* name, double value );
    const char* getModuleName();
    void reset();

private:
    void* genLibrary_;
    bool  compiling_;
    void* (*genWrapperCreateFun_)(double);
    void  (*genWrapperDestroyFun_)();
    void  (*genWrapperResetFun_)(double);
    float (*genWrapperPerformFun_)(float);
    void  (*genWrapperSetParameterFun_)(const char*, double );
    
};

#endif // _CHIRP_FILTER_H_
