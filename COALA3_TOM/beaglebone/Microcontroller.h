//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      (C) Ircam 2016
//      Microcontroller.h
//      programmed by Robert Piechaud
//

#include <stdio.h>

#ifndef _MICROCONTROLLER_H_
#define _MICROCONTROLLER_H_

#define BUFFER_SIZE 64

class Microcontroller
{
private:
    Microcontroller();
    virtual ~Microcontroller();
    
public:
    bool write( const char* message );
    
private:
    void initialize();
    void flush();
    void initComm( void* raw );
    void run();
    
public:
    static Microcontroller* getInstance();
    static void             deleteInstance();
    static void*            start( void* );
    
private:
    static Microcontroller* instance_;
    volatile bool           mustDestroy_;
    char                    output_ [64];
    char                    ack_ [64];
};

#endif // _MICROCONTROLLER_H_
