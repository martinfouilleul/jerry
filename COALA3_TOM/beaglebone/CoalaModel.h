//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      (C) Ircam 2014
//      CoalaModel.h
//      programmed by Robert Piechaud
//

#include <stdio.h>
#include <vector>
#include <string>
#include "maths/Vector.h"
#include "core/ModelInterface.h"
#include "maths/BlockDiagonalMatrix.h"

#ifndef _COALA_MODEL_H_
#define _COALA_MODEL_H_

class RealTimeInterface;
class ControlLoopFacade;
class FilterInterface;
class GenFilter;

class CoalaModel: public ModelInterface
{
public:
    CoalaModel( RealTimeInterface& realtimeManager, ControlLoopFacade& controlLoop );
    virtual ~CoalaModel();

    virtual bool         initialize( bool testMode );
    virtual void         step();
    virtual unsigned int getInputDimension() const { return 1; };
    virtual unsigned int getOutputDimension() const { return 1; };
    virtual void         setCurrentInput( unsigned int inputIndex, float value );
    virtual float        getCurrentOutput( unsigned int outputIndex ) const;
    virtual void         cleanupAfterRun();
    virtual bool         handleMessage( const std::string& message );
    virtual void         serializeStatus( std::stringstream& stream ) const;

    void  startup();
    void  prepareForStartup();
    bool  loaded() const { return loaded_; }
    void  setLoaded( bool status ) { loaded_ = status; }
    
    void  setGain( float gain ) { gain_ = gain; }
    float gain() const { return gain_; }

    bool  recordEnabled() const { return recordOn_; }
    void  saveRecordData();

private:
    void  setCurrentOutput( float value );

private:
    RealTimeInterface&            realtimeManager_;
    ControlLoopFacade&            loop_;
    bool                          loaded_;
    bool                          recordOn_;
    std::vector<FilterInterface*> filters_;
    GenFilter*                    genFilter_;

    float                y_;        //measure of the real system output
    float                u_;
    
    float*               buffer1_;
    float*               buffer2_;
    float*               buffer3_;
    float*               buffer4_;
    long long*           loopTiming_;
    long long*           time_;
    float                gain_;
    long long            t_;
    long long            tBefore_;
    long long            k_;
    long                 recordLimit_;  
};

#endif // _COALA_MODEL_H_
