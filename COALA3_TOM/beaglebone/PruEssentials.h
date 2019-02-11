//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      @ Ircam 2014
//      PruEssentials.h
//      Header file common to C/C++ and ASM codes
//      programmed by Robert Piechaud
//
                                   

#ifndef _PRU_ESSENTIALS_H_
#define _PRU_ESSENTIALS_H_

#define PRU_TIMER_NUM           0
#define	PRU_CONVERTER_NUM       1

#define OFFSET_TIME64           0x20
#define OFFSET_TIME64_BUSY      0x28
#define OFFSET_CLOCK_VALUE      0x10
//#define OFFSET_CLOCK_MULTIPLE   0x1A
#define PRU_REALTIME_NS_PERIOD  40

#define OFFSET_DATAIN           0x30 // = address offset PRU <=> 0x30 = 48 en décimal divisé par 4 dans le .cpp pour y accéder car emplacements de 32 bits sur ram PRU = 4 octets
#define OFFSET_DATAOUT          0x40
#define OFFSET_DATAIN_REQUEST   0x34
#define OFFSET_DATAIN_READY     0x38
#define OFFSET_DATAOUT_READY    0x44 
#define OFFSET_DATAOUT_DONE     0x48 

#endif
