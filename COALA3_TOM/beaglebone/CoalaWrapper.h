//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      © Ircam 2014
//      CoalaWrapper.h
//      programmed by Robert Piechaud
//


#include "core/ControlLoopFacade.h"

#ifndef _COALA_WRAPPER_H_
#define _COALA_WRAPPER_H_

//class ModalToolBox;
class WebServer;
class PrussManager;

class CoalaWrapper: public ControlLoopFacade
{
private:
    CoalaWrapper( const std::string& dir );
    virtual ~CoalaWrapper();

public:
    static CoalaWrapper*      getInstance();
    static CoalaWrapper*      getInstance( const std::string& dir );
    static void               deleteInstance();

    virtual bool              initialize();
    virtual void              run();
    virtual std::string       getVersion() const;
    void                      launch();
    void                      exit();
    void                      identifySystem( char* fileName );
    void                      allowRealTime( bool status );

    bool                      realTimeEnabled() const;
    bool                      recordEnabled() const;
    bool                      shareTimeEnabled() const;
    float                     gain() const;
    const std::string&        workingDirectory() const;
    
    void enableModel( bool status );
    void enableChirp( bool status );
    void enableRecord( bool status );
    void enableShareTime( bool status );
    void enableWebServer( int port );
    void enableOSCServer( int port );
    void setChirpGain( float gain );
    void setGain( float gain );
    void setParam1( float gain );
    void setChirpFrequencies( float startFreq, float endFreq );
    void mustRun( bool status );
    void flushOutData();
    void setModelLoaded( bool status );
    bool handleMessage( const std::string& message );
    void sendMessageToMcu( const char* message );
    void sendMessageToMcu( const std::string& message );
    void sendMessageToMcu( const char* header, int value );
    void recompileGenModule();
  
    std::string getStatus() const;

    long long testGiveTime() const;
    
    static void* webServerHandler( void *arg );
    static void* oscServerHandler( void *arg );

private:
    virtual void              initializeComponents();
    void launchWebServer();
    void launchOSCServer();
    void launchMicrocontrollerServer();

//public:
    //ModalToolbox*             modalToolbox_;  //experimental (for future parameters identification purpose)

private:
    PrussManager*             prussManager_;
    bool                      mustExit_;
    bool                      mustRun_;
    bool                      mustPause_;
    bool                      hasWebServer_;
    bool                      hasOscServer_;
};

#endif // _COALA_WRAPPER_H_

