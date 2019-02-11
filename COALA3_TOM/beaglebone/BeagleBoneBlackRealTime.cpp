//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      @ Ircam 2014
//      BeagleBoneRealTime.ccp
//      programmed by Robert Piechaud
//
//    	This class reflects the real time use
//		of Xenomai (2.6.x) running the BeagleBone Black.
//

#include "BeagleBoneBlackRealTime.h"
#include "PrussManager.h"
#include "core/RealTimeEssentials.h"
#include <stdlib.h>
#include <math.h>
#include <sys/mman.h>
#include <execinfo.h>
#include <errno.h>
#include <time.h>
#include <sys/timeb.h>
#include <native/timer.h>
#include <unistd.h>
#include <stdarg.h>
#include <rtdk.h>

/*
RealTimeInterface* BeagleBoneBlackRealTime::_instance = 0;

RealTimeInterface* BeagleBoneBlackRealTime::getInstance( PrussManager& prussManager )
{
    if ( !_instance )
        _instance = new BeagleBoneBlackRealTime( prussManager );
    return _instance;
}

void BeagleBoneBlackRealTime::deleteInstance()
{
    delete _instance;
    _instance = 0;
}
*/

BeagleBoneBlackRealTime::BeagleBoneBlackRealTime( PrussManager& prussManager ):
    initialized_            ( false ),
    enabled_                ( true ),
    canShareFreeTime_       ( true ),
    lastTimeShare_          ( 0 ),
    prussManager_           ( prussManager )
{
    setupSignalcatcher();
}

BeagleBoneBlackRealTime::~BeagleBoneBlackRealTime()
{
    //NOTHING
}

bool BeagleBoneBlackRealTime::initialize()
{
    if ( !prussManager_.turnOnPrus() )
        return false;
    initialized_ = true;
    return true;
}

long long BeagleBoneBlackRealTime::getHighPrecisionNanosecondTime() const
{
    if ( !prussManager_.getPruSharedMemory() )
        return getCpuNanosecondTime();
    return *((unsigned long long*)(prussManager_.getPruSharedMemory()+OFFSET_TIME64))*PRU_REALTIME_NS_PERIOD;
}

long long BeagleBoneBlackRealTime::getCpuNanosecondTime() const
{
    return (long long) rt_timer_tsc2ns( (SRTIME) rt_timer_tsc());
    /*
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);  //CLOCK_MONOTONIC_RAW
    return (long long) now.tv_nsec + now.tv_sec*1.0e9;
    */
}

void BeagleBoneBlackRealTime::waitUntilTaskCompletion( void* task )
{
    rt_task_join( (RT_TASK*) task );
    printf( "RealTime: real time task completed\n" );
}

void BeagleBoneBlackRealTime::wait( float seconds )
{
    long long nanoSeconds = (long long)((float) seconds*1.0e9);
    long long t0 = getHighPrecisionNanosecondTime(), t1 = t0;
    while ( t1 - t0 < nanoSeconds )
        t1 = getHighPrecisionNanosecondTime();
}

void BeagleBoneBlackRealTime::shareFreeTime( bool status )
{
    canShareFreeTime_ = status;
}
bool BeagleBoneBlackRealTime::canShareFreeTime() const
{
    return canShareFreeTime_;
}

void BeagleBoneBlackRealTime::handleFreeTime( long long freeNanoseconds )
{
    if ( isEnabled() )
    {
        if ( canShareFreeTime_ && freeNanoseconds > 3000 && freeNanoseconds < 500000 )
        {
            //rt_printf( "je dors... %lld\n", freeNanoseconds);
            rt_task_sleep( freeNanoseconds*2 / 3 );
            return;
        }
        else
        {
            long long t = getHighPrecisionNanosecondTime();
            if ( t - lastTimeShare_ > 50000000 )
            {
                lastTimeShare_ = t;
                //rt_printf( "je dors... %lld\n", t);
                rt_task_sleep( 10000 );
            }
        }
    }
}

void* BeagleBoneBlackRealTime::setTaskInRealTime( void(*taskFunction)(void*) )
{
    printf( "RealTime: switching task to 'hard' real time mode\n" );
    system( "echo \"-1\" >> /proc/sys/kernel/sched_rt_runtime_us" );  //60 sec = 60000000, unlimited = -1

    int err = mlockall( MCL_CURRENT | MCL_FUTURE );
    if ( err != 0 )
    {
        printf("\terror %d in mlockall!\n", err);
        return 0;
    }
    rt_print_auto_init(1);
    err = rt_print_init(32767, "mainTask");
    if ( err != 0 )
    {
        printf("\terror %d in rt_print_init!\n", err);
        return 0;
    }

    err = rt_task_spawn( &realtimeTask_, "mainTask", 0, 99, T_JOINABLE|T_FPU|T_WARNSW, taskFunction, NULL );
    if ( err != 0 )
    {
        printf("\terror %d in rt_task_spawn!\n", err);
        return 0;
    }
    return (void*) &realtimeTask_;
}

void BeagleBoneBlackRealTime::enable( bool status )
{
	enabled_ = status;
}

bool BeagleBoneBlackRealTime::isEnabled() const
{
	return enabled_;
}

int BeagleBoneBlackRealTime::realtimePrintf(const char *format, ...)
{
    int ret;
    va_list ap;
    va_start(ap, format);
    if ( isEnabled() )
        ret = rt_printf(format, ap);
    else
        ret = printf(format, ap);
    va_end(ap);
    return ret;
}

static const char* _Signals[] =
{
    "undefined", //SIGDEBUG_UNDEFINED
    "received signal",  //SIGDEBUG_MIGRATE_SIGNAL
    "invoked syscall", //SIGDEBUG_MIGRATE_SYSCALL
    "triggered fault",   //SIGDEBUG_MIGRATE_FAULT
    "affected by priority inversion",  //SIGDEBUG_MIGRATE_PRIOINV
    "missing mlockall",    //SIGDEBUG_NOMLOCK
    "runaway thread (wouf! wouf! says the watchdog...)"   //SIGDEBUG_WATCHDOG
};

void BeagleBoneBlackRealTime::signalCatcher( int sig, siginfo_t *si, void *context )
{
    unsigned int reason = si->si_value.sival_int;
    void *bt[64];
    int nentries;

    printf( "\nSIGDEBUG received, reason %d: %s\n", reason, reason <= SIGDEBUG_WATCHDOG ? _Signals[reason] : "<unknown>" );
    // Dump a backtrace of the frame which caused the switch to secondary mode:
    nentries = backtrace( bt, sizeof(bt) / sizeof(bt[0]) );
    backtrace_symbols_fd( bt, nentries, fileno(stdout) );
}

void BeagleBoneBlackRealTime::setupSignalcatcher()
{
    struct sigaction sa;
    mlockall( MCL_CURRENT | MCL_FUTURE);
    sigemptyset( &sa.sa_mask );
    sa.sa_sigaction = BeagleBoneBlackRealTime::signalCatcher;
    sa.sa_flags = SA_SIGINFO;
    sigaction( SIGDEBUG, &sa, NULL );
}
