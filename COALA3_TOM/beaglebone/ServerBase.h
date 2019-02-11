//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      @ Ircam 2015
//      ServerBase.h
//      programmed by Robert Piechaud
//

#include <stdio.h>
#include <sstream>

#ifndef _SERVER_BASE_H_
#define _SERVER_BASE_H_

class CoalaWrapper;

class ServerBase
{
public:
    ServerBase( CoalaWrapper& coala );
    virtual ~ServerBase();

    virtual void initialize() = 0;
    virtual int  start() = 0;
    virtual void stop() = 0;

protected:
    virtual void handleStartControl();
    virtual void handlePauseControl();
    virtual void handleStopControl();
    virtual void handleExitCoala();
    
    virtual void handleGain( float value );
    virtual void handleParam1( float value );
    virtual void handleSamplePeriod( float microsec );
    virtual void handleTime( float sec );
    virtual void handleFadingTime( float sec );
    virtual void handleChirpBeginFreq( float beginFreq );
    virtual void handleChirpEndFreq( float endFreq );
    
    virtual void handleActiveControl( bool status );
    virtual void handleChirp( bool status );
    virtual void handleRealTime( bool status );
    virtual void handleAutomaticPeriod( bool status );
    virtual void handleInversePhase( bool status );
    virtual void handleRecordData( bool status );
    virtual void handleShareTime( bool status );
    virtual bool handleMessage( const std::string& message );
    void         handleTime( const std::string& message );

protected:
    CoalaWrapper& coala_;
private:
    bool dateSet_;
};

template < typename T > std::string to_string( const T& n )
{
    std::ostringstream stm ;
    stm << n ;
    return stm.str() ;
}


#endif //_SERVER_BASE_H_
