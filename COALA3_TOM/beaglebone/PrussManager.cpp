//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      @ Ircam 2014
//      PrussManager.cpp
//      programmed by Robert Piechaud
//


#include "PrussManager.h"
#include "core/RealTimeInterface.h"
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include "PruEssentials.h"
#include <string>

PrussManager::PrussManager( const std::string& workingDirectory ):
    workingDirectory_       ( workingDirectory ),
    pruTimeNanosecondPeriod_( PRU_REALTIME_NS_PERIOD ),
    pru0Memory_             ( 0 ),
    pru1Memory_             ( 0 ),
    pruSharedMemory_        ( 0 )
{
    //NOTHING
}

PrussManager::~PrussManager()
{
   prussdrv_pru_disable ( 0 );
   prussdrv_pru_disable ( 1 );
}

bool PrussManager::turnOnPrus()
{
    prussdrv_init ();
    printf( "RealTime: initializing PRU 0 (clock services)\r\n" );
    int ret = prussdrv_open( PRU_EVTOUT_0 );
    if ( ret )
    {
     	printf( "prussdrv_open open failed for pru 0\n" );
        return false;
    }
    
    printf( "RealTime: initializing PRU 1 (converter services)\r\n" );
    ret = prussdrv_open( PRU_EVTOUT_1 );
    if ( ret )
    {
     	printf( "prussdrv_open open failed for pru 1\n" );
        return false;
    }

    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
    prussdrv_pruintc_init( &pruss_intc_initdata );

    this->mapPruMemories();
    std::string path = workingDirectory_ + "pruTimer.bin";
    ret = prussdrv_exec_program ( 0, path.c_str() );
    if ( ret )
    {
        printf( "prussdrv_exec_program (pruTimer.bin) failed for pru 0 (path: %s)\n", path.c_str() );
        return false;
    }
    path = workingDirectory_ + "pruConverter.bin";
    ret = prussdrv_exec_program ( 1, path.c_str() );
    if ( ret )
    {
        printf( "prussdrv_exec_program (pruConverter.bin) failed for pru 1\n" );
        return false;
    }
    return true;
}

void PrussManager::mapPruMemories()
{
    prussdrv_map_prumem( PRUSS0_SHARED_DATARAM, &pruSharedMemory_ );
    prussdrv_map_prumem (PRUSS0_PRU0_DATARAM, &pru0Memory_ );
    prussdrv_map_prumem (PRUSS0_PRU1_DATARAM, &pru1Memory_ );
}

void* PrussManager::getPruMemory( unsigned int pruNum )
{
    switch (pruNum)
    {
        case 0:
            return pru0Memory_;
        case 1:
            return pru1Memory_;
        default:
            break;
    }
    return 0;
}

const void* PrussManager::getPruMemory( unsigned int pruNum ) const
{
    switch (pruNum)
    {
     	case 0:
            return pru0Memory_;
        case 1:
            return pru1Memory_;
        default:
            break;
    }
    return 0;
}

