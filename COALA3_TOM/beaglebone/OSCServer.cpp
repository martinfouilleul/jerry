//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      @ Ircam 2015
//      OSCServer.ccp
//      programmed by Robert Piechaud
//

#include "OSCServer.h"
#include "CoalaWrapper.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <math.h>
#include <complex>
#include <pthread.h>

//TODO: true OSC support! http://opensoundcontrol.org/spec-1_0
//                        http://opensoundcontrol.org/spec-1_0-examples   

using namespace std;

OSCServer* OSCServer::instance_ = 0;

OSCServer* OSCServer::getInstance()
{
    if ( instance_ == NULL )
    {
        instance_ = new OSCServer( *CoalaWrapper::getInstance(), 8000 );
    }
    return instance_;
}

OSCServer* OSCServer::getInstance( CoalaWrapper& loop, int port )
{
    if ( instance_ == NULL )
    {
        instance_ = new OSCServer( loop, port );
    }
    return instance_;
}

void OSCServer::deleteInstance()
{
    if ( instance_ )
    {
        delete instance_;
        instance_ = NULL;
    }
    cout << "OSC server terminated!" << endl;
}

OSCServer::OSCServer( CoalaWrapper& loop, int port ):
    ServerBase    ( loop ),
    port_         ( port ),
    socket_     ( 0 )
{
    cout << "COALA OSC server initializing on port " << port << "..." << endl;
    initialize();
}

OSCServer::~OSCServer()
{
    stop();
}

void OSCServer::initialize()
{
    root_ = coala_.workingDirectory() + "../";
    openSocket();
    printf( "OSC server started at port no. \033[92m%d\033[0m with root directory as \033[92m%s\033[0m\n", port_, root_.c_str() );
}

void OSCServer::openSocket()
{   
    struct sockaddr_in socketInfo;
     
    //create a UDP socket
    socket_ = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if ( socket_ == -1)
    {
        perror ( "socket() error" );
        exit( 1 );
    }
    int optval;
    optval = 1;
    if ( setsockopt( socket_, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval) ) != 0 )
    {
        setsockopt( socket_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval) );
        perror ( "setsockopt() error" );
        exit( 1 );
    }
     
    // zero the structure out
    memset( (char *) &socketInfo, 0, sizeof(socketInfo) );
    socketInfo.sin_family = AF_INET;
    socketInfo.sin_port = htons( port_ );
    socketInfo.sin_addr.s_addr = htonl( INADDR_ANY );
     
    //bind socket to port
    if( bind( socket_ , (struct sockaddr*)&socketInfo, sizeof(socketInfo) ) == -1 )
    {
        perror ( "bind() error" );
        //exit( 1 );
    }
}

int OSCServer::start()
{
    cout << "OSCServer: now listening..." << endl;
    socklen_t slen = sizeof(clientInfo_);
    const int bufferSize = 65536;
    char buffer[bufferSize];
    while( 1 )
    {
        memset( buffer, 0, bufferSize );
        //try to receive some data, this is a blocking call
        int sizeIn = recvfrom( socket_, buffer, bufferSize, 0, (struct sockaddr*) &clientInfo_, &slen );
        if ( sizeIn == -1 )
        {
            perror ( "recvfrom() error" );
            exit( 1 );
        }
         
        //printf( "Received %d bytes from %s:%d\n", sizeIn, inet_ntoa( clientInfo_.sin_addr ), ntohs( clientInfo_.sin_port ) );
        //cout << "osc message, size=" << sizeIn << " " << buffer << " (size(str)=" << strlen(buffer) << ")" << endl;
        if ( strcmp(buffer,"#bundle") == 0)
            handleTrueOSCMessage( buffer, sizeIn );
        else if ( handleOSCMessage( buffer ) != 0 )
          break;
    }
    stop();
    return 0;
}

int OSCServer::handleTrueOSCMessage( char* rawOSC, int bufferSize )
{
    int result = 0;
    bool handled = false;
    unsigned int* bundlesize = (unsigned int*) ((char*) rawOSC+16);
    //cout << "sizeof(unsigned short)=" << sizeof(unsigned short) << endl;
    cout << "incoming true OSC message of size: " << *bundlesize << endl;
    
    return result;
}

int OSCServer::handleOSCMessage( char* raw )
{
    int result = 0;
    bool handled = false;
    string message ( raw );
    if ( message != lastMessage_ && message != "/coala/get/status" )
    {
        cout << "incoming OSC message: " << message << endl;
        lastMessage_ = message;
    }
    size_t pos = message.find_first_of( "/coala" );
    if ( pos == 0 )
    {
        string content = message.substr( 6 );
        pos = content.find( "/event" );
        if ( pos == 0 )
        {
            content = content.substr( 6 );
            if ( content == "/start" )
            {
                handled = true;
                string ack = "/response/coala started!";
                sendResponse( ack );
                handleStartControl();
            }
            else if ( content == "/stop" )
            {
                string ack = "/response/coala stopped!";
                sendResponse( ack );
                handleStopControl();
                handled = true;
            }
            else if ( content == "/exit" )
            {
                handled = true;
                handleExitCoala();
                result = 1;
            }
        }
        pos = content.find( "/set" );
        if ( pos == 0 )
        {
            content = content.substr( 4 );
            pos = content.find( "/chirp" );
            if ( pos == 0 )
            {
                content = content.substr( 6 );
                pos = content.find( "/enable" );
                if ( pos == 0 )
                {
                    handled = true;
                    content = content.substr( 8 );
                    int value = 1;
                    if ( content == "0" || content == "false" )
                      value = 0;
                    handleChirp( value );
                    return result;
                    /*
                    string ack;
                    if ( value )
                      ack = "chirp enabled!";
                    else
                      ack = "chirp disabled!";
                    sendResponse( ack );
                    */
                }
                pos = content.find( "/startfreq" );
                if ( pos == 0 )
                {
                    handled = true;
                    float value = atof( content.c_str() + 11 );
                    handleChirpBeginFreq( value );
                    return result;
                }
                pos = content.find( "/endfreq" );
                if ( pos == 0 )
                {
                    handled = true;
                    float value = atof( content.c_str() + 9 );
                    handleChirpEndFreq( value );
                    return result;
                }
            }
            pos = content.find( "/control" );
            if ( pos == 0 )
            {
                content = content.substr( 8 );
                pos = content.find( "/enable" );
                if ( pos == 0 )
                {
                    handled = true;
                    content = content.substr( 8 );
                    int value = 1;
                    if ( content == "0" || content == "false" )
                      value = 0;
                    handleActiveControl( value );
                    return result;
                }
                pos = content.find( "/param1" );
                if ( pos == 0 )
                {
                    handled = true;
                    float value = atof( content.c_str() + 8 );
                    handleParam1( value );
                    return result;
                }
                pos = content.find( "/gain" );
                if ( pos == 0 )
                {
                    handled = true;
                    float value = atof( content.c_str() + 6 );
                    handleGain( value );
                    return result;
                }
            }
            pos = content.find( "/date" );
            if ( pos == 0 )
            {
                cout << "date request: " << content << endl;
                handleTime( content.substr( 6 ) );
                return result;
            }
	        pos = content.find ( "/time" );
            if ( pos == 0 )
            {
                handled = true;
                float value = atof( content.c_str() + 6 );
                handleTime( value );
                return result;
            }
            pos = content.find( "/fadetime" );
            if ( pos == 0 )
            {
                handled = true;
                float value = atof( content.c_str() + 10 );
                handleFadingTime( value );
                return result;
            }
        }
        pos = content.find( "/get" );
        if ( pos == 0 )
        {
            content = content.substr( 4 );
            pos = content.find( "/status" );
            if ( pos == 0 )
            {
                handled = true;
                std::string status = "/status/" + coala_.getStatus();
                sendResponse( status );
                return result;
            }
        }
        else
        {
            coala_.handleMessage( message );
        }
    }
    /*
    if ( handled )
      cout << "(incoming OSC message '" << message << "' handled :-) )" << endl;
    else
      cout << "(incoming OSC message '" << message << "' not handled :-( )" << endl;
    */
    return result;
}

void OSCServer::sendResponse( const std::string& response )
{
    //tcpdump udp -vv -X
    //cout << "sending response: " << response << endl;
    socklen_t slen = sizeof(struct sockaddr_in ); //sizeof( clientInfo_ );
    clientInfo_.sin_port = htons( port_ );
    const int bufferSize = 512;
    char buf[bufferSize];
    memset( buf, 0, bufferSize );
    strcpy( buf, response.c_str() );
    //buf[4] = ',';
    //int length = 8;
    int length = (response.length() + 4) & ~0x3;
    buf[length]=',';
    length += 4;
    
    /*
    cout << "sending response to" << "clientInfo_.sin_port="
         << clientInfo_.sin_port << ", clientInfo_.sin_addr="
         << inet_ntoa( clientInfo_.sin_addr )
         << ", clientInfo_.sin_family=" << clientInfo_.sin_family
         << ", size=" << length
         << endl;
    */
    int sout = sendto( socket_, buf, length, 0, (struct sockaddr*) &clientInfo_, slen );
    if ( sout == -1 )
    {
        perror ( "sendto() error" );
        exit( 1 );
    }
}

void OSCServer::stop()
{
    int result = close( socket_ );
    cout << "  closing socket (port " << port_ << ", result=" << result << ")" << endl;
}

