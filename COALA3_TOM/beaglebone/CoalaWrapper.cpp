//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      © Ircam 2014
//      CoalaWrapper.cpp
//      programmed by Robert Piechaud
//

#include "CoalaWrapper.h"
#include "BeagleBoneBlackRealTime.h"
#include "CoalaModel.h"
#include "PrussManager.h"
#include "SignalConverter.h"
#include "ModalToolbox.h"
#include "Microcontroller.h"
#include "WebServer.h"
#include "OSCServer.h"
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <pthread.h>
#include <sstream>
#include <iostream>

using namespace std;

string ftoa( float value )
{
    char buffer[32];
    sprintf( buffer, "%f", value );
    return string( buffer );
}

CoalaWrapper::CoalaWrapper( const std::string& dir ):
    ControlLoopFacade (),
    prussManager_     ( new PrussManager( dir ) ),
    mustExit_         ( false ),
    hasWebServer_     ( false ),
    hasOscServer_     ( false )
{
    setWorkingDirectory( dir );
    cout << endl << endl << "Coala Wrapper : opening..." << endl;
    cout << "software version: " << getVersion() << endl;
#ifdef NEON_ON
    cout << "Neon optimization on." << endl;
#else
    cout << "Neon optimization off." << endl;
#endif
    this->initializeComponents();
    flushOutData();
}

CoalaWrapper::~CoalaWrapper()
{
    mustExit_ = true;
    realTimeManager_ = 0;
    delete converter_;
    delete model_;
    delete realTimeManager_;
    //delete modalToolbox_;
    delete prussManager_;
    Microcontroller::deleteInstance();
    cout << "Coala Wrapper : closing." << endl << endl;
}

CoalaWrapper* CoalaWrapper::getInstance( const std::string& dir )
{
    if ( instance_ == NULL )
        instance_ = new CoalaWrapper( dir );
    return static_cast<CoalaWrapper*>( instance_ );
}

CoalaWrapper* CoalaWrapper::getInstance()
{
    return static_cast<CoalaWrapper*>( instance_ );
}

void CoalaWrapper::deleteInstance()
{
    WebServer::deleteInstance();
    OSCServer::deleteInstance();
    delete instance_;
}

void CoalaWrapper::initializeComponents()
{
    realTimeManager_    = new BeagleBoneBlackRealTime( *prussManager_ );
    model_              = new CoalaModel( *realTimeManager_, *this );
    converter_          = new SignalConverter( *realTimeManager_, *prussManager_ );
    componentsLoadedNotification();
}

string CoalaWrapper::getVersion() const
{
    return "2.1.0.1";
}

const string& CoalaWrapper::workingDirectory() const
{
    return workingDirectory_;
}

long long CoalaWrapper::testGiveTime() const
{
    if ( realTimeManager_ )
        return realTimeManager_->getHighPrecisionNanosecondTime();
    return 0;
}

//static
void* CoalaWrapper::webServerHandler( void* arg )
{
    
    WebServer* webserver = WebServer::getInstance();
    webserver->start();
    return NULL;
}

void* CoalaWrapper::oscServerHandler( void* arg )
{
    OSCServer* oscServer = OSCServer::getInstance();
    oscServer->start();
    return NULL;
}

void CoalaWrapper::mustRun( bool status )
{
    mustRun_ = status;
}

void CoalaWrapper::run()
{
    if ( !initialize() )
    {
        cout << "Coala Wrapper : initialization problem! :-(" << endl;
        mustRun_ = false;
        return;
    }
    model_->initialize( false );
    adjustIdealSamplingTime();
    sendMessageToMcu("SET_RUNNING=1");
    ControlLoopFacade::run();
    sendMessageToMcu("SET_RUNNING=0");
    mustRun( false );
}

void CoalaWrapper::flushOutData()
{
    system("rm -f ../data/out/*.*");
}

void CoalaWrapper::setModelLoaded( bool status )
{
    ( (CoalaModel*) model_ )->setLoaded( status );
}

void CoalaWrapper::launch()
{
    initialize();
    launchMicrocontrollerServer();
    
    if ( !hasWebServer_ && !hasOscServer_ )
    {
        run();
    }
    else
    {
        if ( hasWebServer_ )
            launchWebServer();
        if ( hasOscServer_ )
            launchOSCServer();
        
        while ( !mustExit_ )
        {
            if ( mustRun_ )
                run();
        }
    }
}


void CoalaWrapper::launchWebServer()
{
    pthread_attr_t attr;
    struct sched_param p;
    pthread_t threadId;
    
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    p.sched_priority = 99;
    pthread_attr_setschedparam(&attr, &p);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    int err = pthread_create( &threadId, &attr, &webServerHandler, NULL );
}

void CoalaWrapper::launchOSCServer()
{
    pthread_attr_t attr;
    struct sched_param p;
    pthread_t threadId;
    
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    p.sched_priority = 99;
    pthread_attr_setschedparam(&attr, &p);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    int err = pthread_create( &threadId, &attr, &oscServerHandler, NULL );
}

void CoalaWrapper::launchMicrocontrollerServer()
{
    pthread_attr_t attr;
    struct sched_param p;
    pthread_t threadId;
    
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    p.sched_priority = 99;
    pthread_attr_setschedparam(&attr, &p);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    int err = pthread_create( &threadId, &attr, &Microcontroller::start, NULL );
}

void CoalaWrapper::identifySystem( char* fileName )
{
    //modalToolbox_->testFFT( fileName );
}

bool CoalaWrapper::initialize()
{
    system( "echo 1 > /sys/class/gpio/gpio66/value" );
    sendMessageToMcu("SET_RUNNING=0");
    return ControlLoopFacade::initialize();
}

void CoalaWrapper::exit()
{
    mustExit_ = true;
    system( "echo 0 > /sys/class/gpio/gpio66/value" );
}

void CoalaWrapper::enableShareTime( bool status )
{
    realTimeManager_->shareFreeTime( status );
}

void CoalaWrapper::enableWebServer( int port )
{
    if ( port )
    {
        if ( WebServer::getInstance( *this, port ) )
            hasWebServer_ = true;
    }
}

void CoalaWrapper::enableOSCServer( int port )
{
    if ( port )
    {
        if ( OSCServer::getInstance( *this, port ) )
        {
            hasOscServer_ = true;
            if ( initialized() )
            {
                launchOSCServer();
            }
        }
    }
    else
    {
        OSCServer::deleteInstance();
        hasOscServer_ = false;
    }
}

void CoalaWrapper::allowRealTime( bool status )
{
    realTimeManager_->enable( status);
}

bool CoalaWrapper::realTimeEnabled() const
{
    realTimeManager_->isEnabled();
}


bool CoalaWrapper::recordEnabled() const
{
    ( (CoalaModel*) model_ )->recordEnabled();
}

bool CoalaWrapper::shareTimeEnabled() const
{
    return realTimeManager_->canShareFreeTime();
}

void CoalaWrapper::enableModel( bool status )
{
    string message = "/coala/set/modalcontrol/enable/";
    message += status?"1":"0";
    model_->handleMessage( message );
}

void CoalaWrapper::enableChirp( bool status )
{
    string message = "/coala/set/chirp/enable/";
    message += status?"1":"0";
    model_->handleMessage( message );
    //sendMessageToMcu("SET_GAIN_PREAMP=10");
}

void CoalaWrapper::enableRecord( bool status )
{
    string message = "/coala/set/record/enable/";
    message += status?"1":"0";
    model_->handleMessage( message );
}

void CoalaWrapper::setChirpFrequencies( float f0, float f1 )
{
    string message = "/coala/set/chirp/startfreq/";
    message += ftoa( f0 );
    model_->handleMessage( message );
    message = "/coala/set/chirp/endfreq/";
    message += ftoa( f1);
    model_->handleMessage( message );
}

void CoalaWrapper::setChirpGain( float gain )
{
    //obsolete
    //( (CoalaModel*) model_ )->setChirpGain( gain );
}

void CoalaWrapper::setGain( float gain )
{
    ( (CoalaModel*) model_ )->setGain( gain );
    //printf( "CoalaWrapper::setGain = %f\n", gain );
    //TODO:
    //string message = "/coala/set/outputgain/";
    //message += ftoa( gain) ;
    //model_->handleMessage( message );
}

void CoalaWrapper::setParam1( float gain )
{
    string message = "/coala/set/modulatedgainfreq/";
    message += ftoa( gain );
    model_->handleMessage( message );
}

float CoalaWrapper::gain() const
{
    return ( (CoalaModel*) model_ )->gain();
}

string CoalaWrapper::getStatus() const
{
    stringstream state;
    string space = "+";
    state << "product=COALA" << space;
    state << "version=" << getVersion() << space;
    state << "bypass=" << (isBypassed()?"true":"false") << space;
    state << "running=" << (isRunning()?"true":"false") << space;
    state << "realtime=" << (realTimeEnabled()?"true":"false") << space;
    state << "share_time=" << (shareTimeEnabled()?"true":"false") << space;
    state << "record=" << (recordEnabled()?"true":"false") << space;
    state << "auto_sample_period=" << (automaticSamplingTime()?"true":"false") << space;
    state << "sample_period=" << getSamplingTime() << space;
    state << "time_limit=" << getTimeLimit() << space;
    state << "output_gain=" << gain() << space;
    state << "smoothing_time=" << getFadingTime() << space;
    state << "oscstatus=" << hasOscServer_ << space;
    state << "webstatus=" << hasWebServer_ << space;
    model_->serializeStatus( state );
    return state.str();
}

bool CoalaWrapper::handleMessage( const string& message )
{
    size_t pos = message.find( "/?bypass=" );
    if ( pos == 0 )
    {
        string value = message.substr( 9 );
        bypass( (bool ) atoi( value.c_str()) );
        return true;
    }
    pos = message.find( "/coala/event/bypass/" );
    if ( pos == 0 )
    {
        string value = message.substr( 20 );
        bypass( (bool ) atoi( value.c_str()) );
        return true;
    }
    pos = message.find( "/coala/osc/enable/" );
    if ( pos == 0 )
    {
        string value = message.substr( 18 );
        bool request = ( bool ) atoi( value.c_str());
        if ( request != hasOscServer_ )
        {
            enableOSCServer( request? 8000:0 );
        }
        return true;
    }
    pos = message.find( "/coala/set/autosampleperiod/" );
    if ( pos == 0 )
    {
        string value = message.substr( 28 );
        bool request = ( bool ) atoi( value.c_str());
            setAutomaticSamplingTime( request );
        return true;
    }
    pos = message.find( "/coala/set/sampleperiod/" );
    if ( pos == 0 )
    {
        string value = message.substr( 24 );
        float request = ( float ) atof( value.c_str());
        setSamplingTime( request );
        return true;
    }
    pos = message.find( "/coala/set/time/" );
    if ( pos == 0 )
    {
        string value = message.substr( 16 );
        float request = ( float ) atof( value.c_str());
        setTimeLimit( request );
        return true;
    }

    if ( message.find( "/coala/mcu/" ) == 0 )
    {
        sendMessageToMcu( message );
        return true;
    }
    if ( isBypassed() )
        return false;
    return model_->handleMessage( message );
}

void CoalaWrapper::sendMessageToMcu( const char* message )
{
    return;
    //TODO: autodetect the presence of MCU!
    //Microcontroller::getInstance()->write( message );
}

void CoalaWrapper::sendMessageToMcu( const std::string& message )
{
    sendMessageToMcu( message.c_str() );
}

void CoalaWrapper::recompileGenModule()
{
    model_->handleMessage( "/coala/gen/recompile" );
}

/*
void CoalaWrapper::sendMessageToMcu( const char* header, int value )
{
    string message ( header );
    message += "=";
    //message += itoa( value );
    sendMessageToMcu( message );
}
*/

