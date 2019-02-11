//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      (C) Ircam 2016
//      Microcontroller.cpp
//      programmed by Robert Piechaud
//

#include "Microcontroller.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>


static pthread_mutex_t gmutex_;


Microcontroller* Microcontroller::instance_ = NULL;

using namespace std;

Microcontroller::Microcontroller():
    mustDestroy_( false )
{
    output_[0] = 0;
    ack_[0] = 0;
    pthread_mutex_init( &gmutex_, NULL );
}

Microcontroller::~Microcontroller()
{
    mustDestroy_ = true;
    while ( mustDestroy_ )
        usleep( 1000 );
    pthread_mutex_destroy( &gmutex_ );
}

Microcontroller* Microcontroller::getInstance()
{
    if ( instance_ == NULL )
        instance_ = new Microcontroller();
    return instance_;
}

void Microcontroller::deleteInstance()
{
    if ( instance_ )
        delete instance_;
}

void Microcontroller::run()
{
    while ( !mustDestroy_ )
    {
        usleep( 50000 );
        flush();
    }
    mustDestroy_ = false;
}

void* Microcontroller::start( void* )
{
    getInstance()->run();
    return NULL;
}

void Microcontroller::initialize()
{
    //NOTHING
}

bool Microcontroller::write( const char* message )
{
    if ( strlen( message ) + 1 >= BUFFER_SIZE || strlen( message ) == 0 )
        return false;
    pthread_mutex_lock( &gmutex_ );
    printf( "write: %s\n", message );
    output_[0] = '>';
    strcpy( output_ + 1, message );
    pthread_mutex_unlock( &gmutex_ );
    return true;
}

void Microcontroller::flush()
{
    pthread_mutex_lock( &gmutex_ );
    if ( strlen( output_ ) > 0 )
    {
        //printf( "Sending message %s via uart! (l=%d) :-)\n", output_, strlen( output_ ) );
        struct termios newtio;   // structures de configuration du port serie
        uint8_t rxcar;
        uint32_t idx = 0;
        // Ouverture et configuration du port serie
        int serial_fd = open("/dev/ttyO1", O_RDWR); // | O_NOCTTY | O_NDELAY);
        if(serial_fd < 0)
        {
            perror("opening serial device\n");
        }
        else
        {
            //printf("/dev/ttyO1 open\n");
            // modify the port configuration
            tcgetattr( serial_fd, &newtio );
            initComm( &newtio );
            tcsetattr( serial_fd, TCSANOW, &newtio );
            if ( ::write( serial_fd, output_, strlen( output_ ) ) <= 0 )
            {
                printf("Message could not be sent via uart! :-(\n");
            }
            else
            {
                //printf("%s message sent via uart! :-)\n", output_ );
                ack_[0] = 0;
                while( true )
                {
                    int res = read( serial_fd, &rxcar, 1 );
                    if(res <= 0)
                    {
                        continue;
                    }
                    if( rxcar == '\n' )
                    {
                        ack_[idx] = rxcar;
                        break;
                    }
                    printf("%c", rxcar);
                    ack_[idx] = rxcar;
                    if(idx < BUFFER_SIZE -1)
                        idx++;
                }
             }
        }
        output_[0] = 0;
    }
    pthread_mutex_unlock( &gmutex_ );
}

void Microcontroller::initComm( void* raw )
{
    struct termios *pts = (struct termios*) raw;
    /* some things we want to set arbitrarily */
    pts->c_lflag &= ~ICANON;
    pts->c_lflag &= ~(ECHO | ECHOCTL | ECHONL);
    pts->c_cflag |= HUPCL;
    pts->c_cc[VMIN] = 1;
    pts->c_cc[VTIME] = 0;
    /* Standard CR/LF handling: this is a dumb terminal.
     * Do no translation:
     * no NL -> CR/NL mapping on output, and
     * no CR -> NL mapping on input.
     */
    pts->c_oflag &= ~ONLCR; /* set NO CR/NL mapping on output */
    /*pts->c_oflag |= ONLCR;
     crnl_mapping = 1;*/
    pts->c_iflag &= ~ICRNL; /* set NO CR/NL mapping on input */
    /* set no flow control by default */
    pts->c_cflag &= ~CRTSCTS;
    pts->c_iflag &= ~(IXON | IXOFF | IXANY);
    /* set hardware flow control by default */
    /*pts->c_cflag |= CRTSCTS;
     pts->c_iflag &= ~(IXON | IXOFF | IXANY);*/
    /* set 115200 bps speed by default */
    cfsetospeed(pts, B115200);
    cfsetispeed(pts, B115200);
}
