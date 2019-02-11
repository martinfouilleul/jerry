//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      @ Ircam 2015
//      WebServer.h
//      programmed by Robert Piechaud
//

#include <stdio.h>
#include <string>
#include "ServerBase.h"

#ifndef _WEBSERVER_H_
#define _WEBSERVER_H_

class WebServer : public ServerBase
{
private:
    WebServer( CoalaWrapper& coala, int port );
    virtual ~WebServer();

public:
    static WebServer* getInstance();
    static WebServer* getInstance( CoalaWrapper& coala, int port );
    static void deleteInstance();

    virtual void initialize();
    virtual int  start();
    virtual void stop();

private:
    void openSocket();
    int  respond( int index );
    void updateClientHtml();

private:
    static WebServer*  instance_;
    int                port_;
    std::string        root_;
    int                listenfd_;
    int*               clients_;
    std::string        clientHtml_;
    bool               busy_;
};

#endif
