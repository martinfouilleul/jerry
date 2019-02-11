//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      @ Ircam 2014
//      ModalToolBox.h
//      programmed by Robert Piechaud
//

#include <stdio.h>

#ifndef _MODAL_TOOLBOX_H_
#define _MODAL_TOOLBOX_H_

class ModalToolbox
{
public:
    ModalToolbox();
    virtual ~ModalToolbox();
    
    void testFFT( char* fileName );
    void outputFFT( float* buffer, long size );
    //void identifySystem();

private:
    //members
};

#endif
