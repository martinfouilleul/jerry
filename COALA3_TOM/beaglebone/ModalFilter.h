//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      (C) Ircam 2014
//      ModalFilter.h
//      programmed by Robert Piechaud
//

#include <stdio.h>
#include "maths/Vector.h"
#include "core/FilterInterface.h"
#include "maths/BlockDiagonalMatrix.h"

#ifndef _MODAL_FILTER_H_
#define _MODAL_FILTER_H_

class ControlLoopFacade;

class ModalFilter: public FilterInterface
{
public:
    ModalFilter( ControlLoopFacade& loop );
    virtual ~ModalFilter();
    
    virtual bool         initialize();
    virtual void         resetForStartup();
    virtual float        step( float input, long long t );
    virtual bool         handleMessage( const std::string& message );
    virtual void         serializeStatus( std::stringstream& stream ) const;
    virtual bool         hasCommand() const { return true; }

    bool  loadContinuousDataFromMatlabFiles();
    void  unload();
    void  startup();
    void  prepareForStartup();
    bool  loaded() const { return loaded_; }
    void  setLoaded( bool status ) { loaded_ = status; }
    void  setModesAmount( unsigned int modes ) { numModes_ = modes; printf ("ModalFilter::setModesAmount (num modes: %d)\n", numModes_ ); }
    unsigned int getModesAmount() const { return numModes_; }

private:
    void  discretize();
    void  setCurrentOutput( float value );
    void  adjustVectorsOrientation();
    void  stepModel();

private:
    unsigned int         numModes_;
    bool                 loaded_;

    BlockDiagonalMatrix  A_;
    BlockDiagonalMatrix  Ad_;
    Vector               B_;
    Vector               Bd_;
    Vector               C_;
    Vector               K_;
    Vector               L_;
    Vector               Ld_;

    Vector               eLd_;
    Vector               X_;
    Vector               X_next_;
    float                Y_;        //estimate of the system output
};

#endif // _MODAL_FILTER_H_
