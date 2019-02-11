//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      @ Ircam 2015
//      WebServer.ccp
//      programmed by Robert Piechaud
//

#include "WebServer.h"
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
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <math.h>
#include <complex>
#include <pthread.h>

using namespace std;

#define CONNMAX 10
#define BYTES 1024

WebServer* WebServer::instance_ = 0;

WebServer* WebServer::getInstance()
{
    if ( instance_ == NULL )
    {
        instance_ = new WebServer( *CoalaWrapper::getInstance(), 10000 );
    }
    return instance_;
}

WebServer* WebServer::getInstance( CoalaWrapper& loop, int port )
{
    if ( instance_ == NULL )
    {
        instance_ = new WebServer( loop, port );
    }
    return instance_;
}

void WebServer::deleteInstance()
{
    if ( instance_ )
    {
        delete instance_;
        instance_ = NULL;
    }
}

WebServer::WebServer( CoalaWrapper& coala, int port ):
    ServerBase    ( coala ),
    port_         ( port ),
    listenfd_     ( 0 ),
    clients_      ( NULL ),
    busy_         ( false )
{
    cout << "COALA webserver initializing on port " << port << "..." << endl;
    initialize();
}

WebServer::~WebServer()
{
    stop();
    delete clients_;
}

void WebServer::initialize()
{
    clients_ = new int[CONNMAX];
    root_ = coala_.workingDirectory() + "../";

    for ( int i = 0; i< CONNMAX; ++i )
        clients_[i] = -1;
    openSocket();
    printf( "Web server started at port no. \033[92m%d\033[0m with root directory as \033[92m%s\033[0m\n", port_, root_.c_str() );
}

void WebServer::openSocket()
{    
    sched_param param;
    int policy;
    pthread_getschedparam(pthread_self(), &policy, &param);
    fprintf(stderr, "priority = %d, policy = %d\n", param.sched_priority, policy);
    struct addrinfo hints, *res, *p;

    string port = to_string( port_ );

    // getaddrinfo for host
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo( NULL, port.c_str(), &hints, &res) != 0)
    {
        perror ("getaddrinfo() error");
        exit(1);
    }
    // socket and bind
    for ( p = res; p!= NULL; p = p->ai_next )
    {
        listenfd_ = socket( p->ai_family, p->ai_socktype, 0 );
        if ( listenfd_ == -1 )
            continue;
        int optval = 1;
        setsockopt( listenfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval );
        if ( bind( listenfd_, p->ai_addr, p->ai_addrlen ) == 0 )
            break;
    }
    if ( p == NULL )
    {
        perror ("socket() or bind()");
        exit(1);
    }
    freeaddrinfo( res );

    // listen for incoming connections
    if ( listen( listenfd_, 1000000 ) != 0 )
    {
        perror( "listen() error" );
        exit( 1 );
    }
}

int WebServer::start()
{
    // ACCEPT connections
    int slot = 0;
    while (1)
    {
        struct sockaddr_in clientaddr;
        socklen_t addrlen = sizeof( clientaddr );
        clients_[slot] = accept( listenfd_, (struct sockaddr *) &clientaddr, &addrlen );
        if ( clients_[slot] < 0 )
            printf( "accept() error" );
        else
        {
            //if ( fork() == 0 )
            {
                if ( respond(slot) != 0 )
                  break;
                //exit(0);
            }
        }

        while ( clients_[slot] != -1 )
          slot = (slot+1)%CONNMAX;
    }
    stop();
    return 0;
}

int WebServer::respond( int n )
{
    int result = 0;
    if ( busy_ )
        return result;
    const int bufferSize = 1000000;
    char buffer[bufferSize], *reqline[3], data_to_send[BYTES];
    int rcvd, fd, bytes_read;

    memset( (void*)buffer, (int)'\0', bufferSize );

    rcvd = recv( clients_[n], buffer, bufferSize, 0); // MSG_WAITALL

    if ( rcvd < 0 )    // receive error
        fprintf( stderr,("recv() error\n") );
    else if ( rcvd == 0 )    // receive socket closed
        fprintf( stderr,"Client disconnected upexpectedly.\n" );
    else    // message received
    {
        if (rcvd > 400 && coala_.verbose() )
          printf("size: %d\n %s", rcvd, buffer);
        istringstream rawcontent( buffer );
        reqline[0] = strtok (buffer, " \t\n");
        reqline[1] = strtok (NULL, " \t");
        reqline[2] = strtok (NULL, " \t\n");
  
        string content = reqline[1];
        if ( strncmp(reqline[0], "GET\0", 4)==0 )
        {
            if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 )
            {
                write(clients_[n], "HTTP/1.0 400 Bad Request\n", 25);
            }
            else
            {
                bool done = false;
                bool noContentToReturn = false;
                string headerforClient = "HTTP/1.0 200 OK\n\n";
                int pos = string::npos;
                if ( content.find( "bypass=" ) != string::npos )
                {
                    handleMessage( content );
                    noContentToReturn = true;
                }
                else if ( content.find( "activecontrolon=" ) != string::npos )
                {   
                    pos = content.find( "activecontrolon=" );
                    int value = atoi( content.c_str() + pos + 16 );
                    handleActiveControl( value );
                    noContentToReturn = true;
                }
                else if ( content.find( "chirpon=" ) != string::npos )
                {   
                    pos = content.find( "chirpon=" );
                    int value = atoi( content.c_str() + pos + 8 );
                    handleChirp( value );
                    noContentToReturn = true;
                }
                else if ( content.find( "realtimeon=" ) != string::npos )
                {   
                    pos = content.find( "realtimeon=" );
                    int value = atoi( content.c_str() + pos + 11 );
                    handleRealTime( value );
                    noContentToReturn = true;
                }
                else if ( content.find( "autoperiodon=" ) != string::npos )
                {   
                    pos = content.find( "autoperiodon=" );
                    int value = atoi( content.c_str() + pos + 13 );
                    handleAutomaticPeriod( value );
                    noContentToReturn = true;
                }
                else if ( content.find( "inversephaseon=" ) != string::npos )
                {   
                    pos = content.find( "inversephaseon=" );
                    int value = atoi( content.c_str() + pos + 15 );
                    handleInversePhase( value );
                    noContentToReturn = true;
                }
                else if ( content.find( "recorddataon=" ) != string::npos )
                {   
                    pos = content.find( "recorddataon=" );
                    int value = atoi( content.c_str() + pos + 13 );
                    handleRecordData( value );
                    noContentToReturn = true;
                }
                else if ( content.find( "sharetimeon=" ) != string::npos )
                {   
                    pos = content.find( "sharetimeon=" );
                    int value = atoi( content.c_str() + pos + 12 );
                    handleShareTime( value );
                    noContentToReturn = true;
                }
                else if ( content.find( "exit" ) != string::npos )
                {
                    handleExitCoala();
                    noContentToReturn = true;
                    result = 1;
                }
                else if ( content.find( "timeval=" ) != string::npos )
                {
                    pos = content.find( "timeval=" );
                    float value = atof( content.c_str() + pos + 8 );
                    noContentToReturn = true;
                    handleTime( value );
                }
                else if ( content.find( "fadeval=" ) != string::npos )
                {
                    pos = content.find( "fadeval=" );
                    float value = atof( content.c_str() + pos + 8 );
                    noContentToReturn = true;
                    handleFadingTime( value );
                }
                else if ( content.find( "periodval=" ) != string::npos )
                {
                    pos = content.find( "periodval=" );
                    float value = atof( content.c_str() + pos + 10 );
                    noContentToReturn = true;
                    handleSamplePeriod( value );
                }
                else if ( content.find( "gainval=" ) != string::npos )
                {
                    pos = content.find( "gainval=" );
                    float value = atof( content.c_str() + pos + 8 );
                    noContentToReturn = true;
                    handleGain( value );
                }
                else if ( content.find( "param1val=" ) != string::npos )
                {
                    pos = content.find( "param1val=" );
                    float value = atof( content.c_str() + pos + 10 );
                    noContentToReturn = true;
                    handleParam1( value );
                }
                else if ( content.find( "chirpbeginval=" ) != string::npos )
                {
                    pos = content.find( "chirpbeginval=" );
                    float value = atof( content.c_str() + pos + 14 );
                    noContentToReturn = true;
                    handleChirpBeginFreq( value );
                }
                else if ( content.find( "chirpendval=" ) != string::npos )
                {
                    pos = content.find( "chirpendval=" );
                    float value = atof( content.c_str() + pos + 12 );
                    noContentToReturn = true;
                    handleChirpEndFreq( value );
                }
                else if ( content.find( "periodval=" ) != string::npos )
                {   
                    pos = content.find( "periodval=" );
                    float value = atof( content.c_str() + pos + 10 );
                    noContentToReturn = true;
                    handleSamplePeriod( value );
                }
                else if ( content.find( "date=" ) != string::npos )
                {   
                    pos = content.find( "date=" );
                    content = content.substr( pos + 5 );
                    string search = "%20";
                    string replace = " ";
                    size_t pos = 0;
                    while( ( pos = content.find( search, pos)) != string::npos )
                    {
                         content.replace( pos, search.length(), replace );
                         pos += replace.length();
                    }
                    handleTime( content );
                    done = true;
                }
                else if ( content.find( "start" ) != string::npos )
                {   
                    handleStartControl();
                    noContentToReturn = true;
                }
                else if ( content.find( "stop" ) != string::npos )
                {   
                    handleStopControl();
                    noContentToReturn = true;
                }
                else if ( content.find( "pause" ) != string::npos )
                {
                    handlePauseControl();
                    noContentToReturn = true;
                }
                else if ( content.find( "coalastatusrequest" ) != string::npos )
                {
                    string status = coala_.getStatus();
                    string header = "HTTP/1.0 200 OK\n";
                    header += "Content-Type: text/html\n\n";
                    send( clients_[n], header.c_str(), header.size(), 0 );
                    write( clients_[n], status.c_str(), status.size() );
                    done = true;
                }
                else if ( content.find( "datarequest=outdata" ) != string::npos )
                {
                    //busy_ = true;
                    mkdir( "/tmp", 0755 );
                    cout << "Sending out data to client..." << endl;
                    string path = "chmod -R 777 " + root_ + "data/out";
	                system( path.c_str() );
                    path = "zip -rj /tmp/coala_data.zip " + root_ + "data/out";
                    cout << "\tcompressing..." << endl;
                    system( path.c_str() );
                    path = "/tmp/coala_data.zip";
                    if ( ( fd = open(path.c_str(), O_RDONLY)) != -1 )    //FILE FOUND
                    {
                        struct stat info;
                        fstat( fd, &info );
                        if ( coala_.verbose() )
                            cout << "\tzip file size: " << info.st_size << endl;
                        string header = "HTTP/1.0 200 OK\n";
                        header += "Content-Type: application/octet-stream\n";
                        header += "Content-Transfer-Encoding: binary\n";
                        header += "Content-Length: " + to_string( info.st_size );
                        header += "Set-Cookie: fileDownload=true; path=/\n";
                        header += "Content-Disposition: attachment; filename=\"coala_data.zip\"\n\n";
                        cout << "\tsending..." << endl;
                        send( clients_[n], header.c_str(), header.size(), 0 );
                        while ( ( bytes_read = read( fd, data_to_send, BYTES)) > 0 )
			            {
                            int result = send( clients_[n], data_to_send, bytes_read, MSG_NOSIGNAL );
			            }
                    }
                    else
                        write( clients_[n], "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
                    system( "rm -f /tmp/coala_data.zip" );
                    //busy_ = false;
                    done = true;
                }
                else if ( content.find( "datarequest=frequencyresponse" ) != string::npos )
                {
                    cout << "Sending response data to client (for client fft display...)" << endl;
                    string adcPath = root_+ "data/out/adc.txt";
                    string excitationPath = root_+ "data/out/excitation.txt";
                    int fd1 = open( adcPath.c_str(), O_RDONLY );
                    int fd2 = open( excitationPath.c_str(), O_RDONLY );
                    if ( fd1 != -1 && fd2 != -1 )
                    {
                        string header = "HTTP/1.0 200 OK\n\n";
                        send( clients_[n], header.c_str(), header.size(), 0 );
                        while ( ( bytes_read = read( fd1, data_to_send, BYTES)) > 0 )
                            write( clients_[n], data_to_send, bytes_read );
                        string tag = "\nexcitation\n";
                        write( clients_[n], tag.c_str(), tag.length() );
                        while ( ( bytes_read = read( fd2, data_to_send, BYTES)) > 0 )
                            write( clients_[n], data_to_send, bytes_read );
                    }
                    else
                       write( clients_[n], "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
                    close( fd1);
                    close( fd2);
                    done = true;
                }
                else
                {
                    coala_.handleMessage( content );
                }
                if ( !done )
                {
                    string localpath = content;
                    pos = localpath.find_first_of( '?');
                    if ( pos >= 0 )
                      localpath = localpath.substr( 0, pos - 1 );
                    if ( localpath.size() <= 2 )
                      localpath = "/index.html";
                    localpath = root_ + "webserver" + localpath;
                    if ( coala_.verbose() )
                        cout << "request: " << content << " (path: " << localpath << ")" << endl;

                    if ( localpath == "index.html" )
                    {
                      updateClientHtml();
                      if ( noContentToReturn )
                          headerforClient = "HTTP/1.0 204 No Content\n\n";
                      send( clients_[n], headerforClient.c_str(), headerforClient.size(), 0 );
                      if ( !noContentToReturn )
                          write ( clients_[n], clientHtml_.c_str(), clientHtml_.size() + 1 );
                    }
                    else
                    {
                      if ( ( fd = open( localpath.c_str(), O_RDONLY ) ) !=-1 )
                      {
                          send( clients_[n], headerforClient.c_str(), headerforClient.size(), 0 );
                          while ( ( bytes_read = read(fd, data_to_send, BYTES ) ) > 0 )
                            write ( clients_[n], data_to_send, bytes_read );
                      }
                      else
                          write(clients_[n], "HTTP/1.0 404 Not Found\n", 23);
                    }
                }
            }
        }
        else if ( strncmp(reqline[0], "POST\0", 5)==0 )
        {
            if ( coala_.verbose() )
                cout << "post size:" << rawcontent.str().length() << endl;
            if ( content.find( "dataupload=matlab" ) != string::npos )
            {
                string line;
                int count = 0;
                string inDir = root_ + "data/in/";
                mkdir(inDir.c_str(), 0755 );
                while ( getline( rawcontent, line ) )
                {
                    int pos = line.find( "startoffile=\"" );  
                    if ( pos != string::npos )
                    {
                        ++count;
                        pos += 13;
                        int close = line.find( "\"", pos );
                        string fileName = line.substr( pos, close-pos );
                        if ( coala_.verbose() )
                            cout << "file name: " << fileName << endl;
                        getline( rawcontent, line );
                        string path = inDir + fileName;
                        ofstream newMatrix ( path.c_str(), ofstream::binary );
                        while ( line.length() && line.find( "endoffile" ) == string::npos  )
                        {
                            newMatrix << line << endl;
                            //cout << "#" << count << " (size=" << line.length() << "): " << line << endl;
                            getline( rawcontent, line );
                        }
                        newMatrix.close();
                    }
                }
                if ( count == 0 )
                  cout << "No matlab file could be uploaded! :-( " << endl;
                else if ( count == 1 )
                  cout << "One matlab file successfully uploaded! :-) " << endl;
                else
                  cout << count << " matlab files successfully uploaded! :-) " << endl;
                coala_.flushOutData();
                coala_.setModelLoaded( false );
                string headerforClient = "HTTP/1.0 204 No Content\n\n";
                send( clients_[n], headerforClient.c_str(), headerforClient.size(), 0 );
            }
            else if ( content.find( "dataupload=gen_h" ) != string::npos )
            {
                string line;
                int count = 0;
                string inDir = "/usr/src/coala/gen/";
                mkdir(inDir.c_str(), 0755 );
                //cout << rawcontent.str() << endl;
                const char* boundaryinfo = strstr( rawcontent.str().c_str(), "boundary=" );
                if ( boundaryinfo )
                {
                    char boundary [64];
                    int n = sscanf( boundaryinfo, "boundary=%s", boundary );
                    cout << "boundary: " << boundary << endl;
                    const char* pos = strstr( rawcontent.str().c_str(), "filename=\"gen_exported.h\"" );
                    pos += 28;
                    const char* lastpos = strstr( pos, boundary );
                    FILE * codefile = fopen ( "/usr/src/coala/gen/gen_exported.h", "wb");
                    fwrite ( pos , sizeof(char), (size_t)(lastpos - pos - 2), codefile);
                    fclose ( codefile );
                    cout << "gen.h  successfully uploaded! :-) " << endl;
                }
                else
                    cout << "gen.h could not be uploaded! :-( " << endl;
                string headerforClient = "HTTP/1.0 204 No Content\n\n";
                send( clients_[n], headerforClient.c_str(), headerforClient.size(), 0 );
            }
            else if ( content.find( "dataupload=gen_cpp" ) != string::npos )
            {
                string line;
                int count = 0;
                string inDir = "/usr/src/coala/gen/";
                mkdir(inDir.c_str(), 0755 );
                const char* boundaryinfo = strstr( rawcontent.str().c_str(), "boundary=" );
                if ( boundaryinfo )
                {
                    char boundary [64];
                    int n = sscanf( boundaryinfo, "boundary=%s", boundary );
                    cout << "boundary: " << boundary << endl;
                    const char* pos = strstr( rawcontent.str().c_str(), "filename=\"gen_exported.cpp\"" );
                    pos += 30;
                    const char* lastpos = strstr( pos, boundary );
                    FILE * codefile = fopen ( "/usr/src/coala/gen/gen_exported.cpp", "wb");
                    fwrite ( pos , sizeof(char), (size_t)(lastpos - pos - 2), codefile);
                    fclose ( codefile );
                    cout << "gen.cpp  successfully uploaded! :-) " << (lastpos - pos - 2) << endl;
                }
                else
                    cout << "gen.cpp could not be uploaded! :-( " << endl;
                string headerforClient = "HTTP/1.0 204 No Content\n\n";
                send( clients_[n], headerforClient.c_str(), headerforClient.size(), 0 );
                cout << "webserver waiting for a keystroke..." << getchar();
                coala_.recompileGenModule();
            }
        }
    }

    //Closing SOCKET
    shutdown (clients_[n], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
    close(clients_[n]);
    clients_[n]=-1;
    return result;
}

void WebServer::stop()
{
    int result = close( listenfd_ );
    cout << "  closing socket (port " << port_ << ", result=" << result << ")" << endl;
}

//This should be handled in client side Javascript!!
void WebServer::updateClientHtml()
{
    std::string path = root_ + "webserver/index.html";
    ifstream html( path.c_str() );
    if ( html )
    {
        stringstream buffer;
        buffer << html.rdbuf();
        html.close();
        clientHtml_ = buffer.str();
        size_t pos = 0;
        /*
        if ( coala_.activeControlEnabled() )
        {
            pos = clientHtml_.find( "id=\"activecontrolcheckbox\"" );
            if ( pos != string::npos )
            { 
                pos += 26;
                clientHtml_.replace( pos, 0, " checked" );
            }
        }
        if ( coala_.chirpEnabled() )
        {
            pos = clientHtml_.find( "id=\"chirpcheckbox\"", pos );
            if ( pos != string::npos )
            { 
                pos += 18;
                clientHtml_.replace( pos, 0, " checked" );
            }
        }
        */
        if ( coala_.realTimeEnabled() )
        {
            pos = clientHtml_.find( "id=\"realtimecheckbox\"", pos );
            if ( pos != string::npos )
            { 
                pos += 21;
                clientHtml_.replace( pos, 0, " checked" );
            }
        }
        if ( coala_.automaticSamplingTime() )
        {
            pos = clientHtml_.find( "id=\"autoperiodcheckbox\"", pos );
            if ( pos != string::npos )
            { 
                pos += 23;
                clientHtml_.replace( pos, 0, " checked" );
            }
        }
        if ( coala_.recordEnabled() )
        {
            pos = clientHtml_.find( "id=\"recorddatacheckbox\"", pos );
            if ( pos != string::npos )
            { 
                pos += 23;
                clientHtml_.replace( pos, 0, " checked" );
            }
        }
        if ( coala_.shareTimeEnabled() )
        {
            pos = clientHtml_.find( "id=\"sharetimecheckbox\"", pos );
            if ( pos != string::npos )
            { 
                pos += 22;
                clientHtml_.replace( pos, 0, " checked" );
            }
        }
        float gain = coala_.gain();
        pos = clientHtml_.find( "id=\"gainslider\" value=\"", pos );
        if ( pos != string::npos )
        {
            pos += 23;
            size_t nextpos = clientHtml_.find( "\"", pos );
            if ( pos != string::npos )
            {
                clientHtml_.replace( pos, nextpos-pos, to_string(gain) );
                pos = clientHtml_.find( "gainval\">", pos );
                if ( pos != string::npos )
                {
                    pos += 9;
                    nextpos = clientHtml_.find( "<", pos );
                    if ( nextpos != string::npos )
                        clientHtml_.replace( pos, nextpos-pos, to_string(gain) );
                }
            }
        }
        float period = coala_.getSamplingTime()*1.0e6;
        pos = clientHtml_.find( "id=\"periodslider\" value=\"", pos );
        if ( pos != string::npos )
        {
            pos += 25;
            size_t nextpos = clientHtml_.find( "\"", pos );
            if ( pos != string::npos )
            {
                clientHtml_.replace( pos, nextpos-pos, to_string(period) );
                //gainlbl">
                pos = clientHtml_.find( "periodval\">", pos );
                if ( pos != string::npos )
                {
                    pos += 11;
                    nextpos = clientHtml_.find( "<", pos );
                    if ( nextpos != string::npos )
                        clientHtml_.replace( pos, nextpos-pos, to_string(period) );
                }
            }
        }
        float time = coala_.getTimeLimit();
        pos = clientHtml_.find( "id=\"timeslider\" value=\"", pos );
        if ( pos != string::npos )
        {
            pos += 23;
            size_t nextpos = clientHtml_.find( "\"", pos );
            if ( pos != string::npos )
            {
                clientHtml_.replace( pos, nextpos-pos, to_string(time) );
                //gainlbl">
                pos = clientHtml_.find( "timeval\">", pos );
                if ( pos != string::npos )
                {
                    pos += 9;
                    nextpos = clientHtml_.find( "<", pos );
                    if ( nextpos != string::npos )
                        clientHtml_.replace( pos, nextpos-pos, to_string(time) );
                }
            }
        }
        /*
        float chirpBegin = coala_.chirpBegin();
        pos = clientHtml_.find( "id=\"chirpbeginslider\" value=\"", pos );
        if ( pos != string::npos )
        {
            pos += 29;
            size_t nextpos = clientHtml_.find( "\"", pos );
            if ( pos != string::npos )
            {
                clientHtml_.replace( pos, nextpos-pos, to_string(chirpBegin) );
                //gainlbl">
                pos = clientHtml_.find( "chirpbeginval\">", pos );
                if ( pos != string::npos )
                {
                    pos += 15;
                    nextpos = clientHtml_.find( "<", pos );
                    if ( nextpos != string::npos )
                        clientHtml_.replace( pos, nextpos-pos, to_string(chirpBegin) );
                }
            }
        }
        float chirpEnd = coala_.chirpEnd();
        pos = clientHtml_.find( "id=\"chirpendslider\" value=\"", pos );
        if ( pos != string::npos )
        {
            pos += 27;
            size_t nextpos = clientHtml_.find( "\"", pos );
            if ( pos != string::npos )
            {
                clientHtml_.replace( pos, nextpos-pos, to_string(chirpEnd) );
                //gainlbl">
                pos = clientHtml_.find( "chirpendval\">", pos );
                if ( pos != string::npos )
                {
                    pos += 13;
                    nextpos = clientHtml_.find( "<", pos );
                    if ( nextpos != string::npos )
                        clientHtml_.replace( pos, nextpos-pos, to_string(chirpEnd) );
                }
            }
        }
        */
    }
}

