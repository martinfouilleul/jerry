//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      @ Ircam 2015
//      ServerBase.ccp
//      programmed by Robert Piechaud
//

#include "ServerBase.h"
#include "CoalaWrapper.h"
#include <iostream>
#include <stdlib.h>

using namespace std;


ServerBase::ServerBase( CoalaWrapper& loop ):
    coala_    ( loop ),
    dateSet_  ( false )
{
    //NOTHING
}

ServerBase::~ServerBase()
{
    //NOTHING
}

void ServerBase::handleStartControl()
{
   coala_.mustRun( true );
}

void ServerBase::handleExitCoala()
{
   cout << "Leaving COALA environment!" << endl;
   coala_.exit();
}

void ServerBase::handlePauseControl()
{
    //TODO
    //coala_.pause();
}

void ServerBase::handleStopControl()
{
   coala_.stop();
}

void ServerBase::handleGain( float value )
{
    coala_.setGain( value );
    //TODO:
    //string message = "/coala/set/globalgain/";
    //coala_.handleMessage( message );
}

void ServerBase::handleParam1( float value )
{
    coala_.setParam1( value );
}

void ServerBase::handleSamplePeriod( float microsec )
{
    coala_.setSamplingTime( microsec*1.0e-6 );
}

void ServerBase::handleTime( float sec )
{
    coala_.setTimeLimit( sec );
}

void ServerBase::handleFadingTime( float sec )
{
    coala_.setFadingTime( sec );
}

void ServerBase::handleChirpBeginFreq( float freq )
{
    coala_.setChirpFrequencies( freq, 0 );
}

void ServerBase::handleChirpEndFreq( float freq )
{
    coala_.setChirpFrequencies( 0, freq );
}

void ServerBase::handleActiveControl( bool status )
{
    coala_.enableModel( status );
}

void ServerBase::handleChirp( bool status )
{
    coala_.enableChirp( status );
}

void ServerBase::handleRealTime( bool status )
{
    coala_.allowRealTime( status );
}

void ServerBase::handleAutomaticPeriod( bool status )
{
    coala_.setAutomaticSamplingTime( status );
}

void ServerBase::handleInversePhase( bool status )
{
    //coala_.inversePhase( status );
}

void ServerBase::handleRecordData( bool status )
{
    coala_.enableRecord( status );
}

void ServerBase::handleShareTime( bool status )
{
    coala_.enableShareTime( status );
}

bool ServerBase::handleMessage( const string& message )
{
    return coala_.handleMessage( message );
}

void ServerBase::handleTime( const string& date )
{
    if ( !dateSet_ )
    {
      string command ( "date -s \"" );
      command += date + "\"";
      //cout << "setting date from OSC message to ";
      system( command.c_str() );
      dateSet_ = true;
    }
}
