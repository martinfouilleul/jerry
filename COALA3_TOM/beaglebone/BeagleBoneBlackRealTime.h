//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      @ Ircam 2014
//      BeagleBoneBlackRealTime.h
//      programmed by Robert Piechaud
//
//    	This class reflects the real time use
//		of Xenomai (2.6.x) running the BeagleBone Black.
//

#include <stdio.h>
#include <native/task.h>
#include <rtdk.h>
#include <signal.h>
#include <string>
#include "core/RealTimeInterface.h"

#ifndef _BEAGLEBONE_BLACK_REALTIME_H_
#define _BEAGLEBONE_BLACK_REALTIME_H_

class PrussManager;

class BeagleBoneBlackRealTime: public RealTimeInterface
{
public:
    BeagleBoneBlackRealTime( PrussManager& prussManager );
    virtual ~BeagleBoneBlackRealTime();

public:
    //static RealTimeInterface* getInstance( PrussManager& prussManager );
    //static void            deleteInstance();

    virtual bool      initialize();
    virtual void*     setTaskInRealTime( void(*taskFunction)(void*) );
    virtual void      wait( float seconds );
    virtual long long getHighPrecisionNanosecondTime() const;
    virtual long long getCpuNanosecondTime() const;
    virtual void      waitUntilTaskCompletion( void* task );
    virtual void      enable( bool status );
    virtual bool      isEnabled() const;
    virtual void      shareFreeTime( bool status );
    virtual bool      canShareFreeTime() const;
    virtual void      handleFreeTime( long long freeNanoseconds );
    virtual int       realtimePrintf(const char *format, ...);

    //PrussManager&     pruss() { return *prussManager_; };

private:
   void         setupSignalcatcher();
   static void  signalCatcher( int sig, siginfo_t *si, void *context );

private:
   //static RealTimeInterface* _instance;
  
   bool             initialized_;
   bool             enabled_;
   bool             canShareFreeTime_;
   long long        lastTimeShare_;
   PrussManager&    prussManager_;
   RT_TASK          realtimeTask_;
};

#endif //_BEAGLEBONE_BLACK_REALTIME_H_

