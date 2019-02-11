//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      @ Ircam 2014
//      PrussManager.h
//      programmed by Robert Piechaud
//

#include <stdio.h>
#include <native/task.h>
#include <rtdk.h>
#include <signal.h>
#include "PruEssentials.h"
#include <string>

#ifndef _PRUSS_MANAGER_H_
#define _PRUSS_MANAGER_H_

class RealTimeInterface;

class PrussManager
{
public:
    PrussManager( const std::string& workingDirectory );
    virtual ~PrussManager();

    bool         turnOnPrus();
    void         test();

    void*        getPruMemory( unsigned int pruNum );
    const void*  getPruMemory( unsigned int pruNum ) const;
    void*        getPruSharedMemory() { return pruSharedMemory_; } 
    const void*  getPruSharedMemory() const { return pruSharedMemory_; }

private:
    void         mapPruMemories();

private:
    const std::string       workingDirectory_;
    unsigned int            pruTimeNanosecondPeriod_;
    void*                   pru0Memory_;
    void*                   pru1Memory_;
    void*                   pruSharedMemory_;
};

#endif // _PRUSS_MANAGER_H_

