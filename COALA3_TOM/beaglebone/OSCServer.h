//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      @ Ircam 2015
//      OSCServer.h
//      programmed by Robert Piechaud
//

#include <stdio.h>
#include <string>
#include <arpa/inet.h>
#include "ServerBase.h"

#ifndef _OSC_SERVER_H_
#define _OSC_SERVER_H_

class OSCServer : public ServerBase
{
private:
    OSCServer( CoalaWrapper& coala, int port );
    virtual ~OSCServer();

public:
    static OSCServer* getInstance();
    static OSCServer* getInstance( CoalaWrapper& coala, int port );
    static void deleteInstance();

    virtual void initialize();
    virtual int  start();
    virtual void stop();

private:
    void openSocket();
    int  handleOSCMessage( char* message );
    int  handleTrueOSCMessage( char* rawOSC, int bufferSize );
    void sendResponse( const std::string& response );

private:
    static OSCServer*  instance_;
    int                port_;
    std::string        root_;
    int                socket_;
    struct sockaddr_in clientInfo_;
    std::string        lastMessage_;
};

#endif //_OSC_SERVER_H_
