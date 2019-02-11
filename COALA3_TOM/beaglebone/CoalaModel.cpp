//
//      .-. .-.   .-. .-. . . .-. .-. .-. .
//      |(   |    |   | | |\|  |  |(  | | |
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-'
//
//      (C) Ircam 2014
//      CoalaModel.ccp
//      programmed by Robert Piechaud
//

#include "CoalaModel.h"
#include "ModalToolbox.h"
#include "CoalaWrapper.h"
#include "BiquadFilter.h"
#include "ChirpFilter.h"
#include "ModalFilter.h"
//#include "PseudoModalSynth.h"
#include "ModulatedGainFilter.h"
//#include "FFTProcessing.h"
#include "GenFilter.h"
#include "maths/BlockDiagonalMatrix.h"
#include "maths/SquareMatrix.h"
#include "maths/Vector.h"
#include "core/RealTimeInterface.h"
#include "core/ControlLoopFacade.h"
#include "ActiveControlFilter.h"
#include <stdlib.h>
#include <cstdlib>
#include <iostream>
#include <rtdk.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>
#include "SignalConverter.h"
#include <linux/input.h>
#include <fcntl.h>
#include <dlfcn.h>

#define MAX_SAMPLE_BUFFER 5000000
#define BUFLEN 1024
#define FFTLEN 512
#define PARTIALS 10

using namespace std;

CoalaModel::CoalaModel( RealTimeInterface& realtimeManager, ControlLoopFacade& controlLoop ):
    realtimeManager_ ( realtimeManager ),
    loop_            ( controlLoop ),
    loaded_          ( false ),
    recordOn_        ( false ),
    genFilter_       ( NULL ),
    y_               ( 0 ),
    u_               ( 0 ),
    buffer1_         ( NULL ),
    buffer2_         ( NULL ),
    buffer3_         ( NULL ),
    buffer4_         ( NULL ),
    loopTiming_      ( NULL ),
    time_            ( NULL ),
    gain_            ( 0.15 ),
    t_               ( 0 ),
    k_               ( 0 ),
    recordLimit_     ( MAX_SAMPLE_BUFFER )
{
    filters_.push_back( new ModalFilter( loop_ ) );
    filters_.push_back( new ModulatedGainFilter( loop_ ) );
    filters_.push_back( new BiquadFilter( loop_ ) );
    filters_.push_back( new ChirpFilter( loop_ ) );
    genFilter_ = new GenFilter( loop_ );
    filters_.push_back( genFilter_ );

    filters_.push_back(new ActiveControlFilter(loop_));
}

CoalaModel::~CoalaModel()
{
    vector<FilterInterface *>::iterator it = filters_.begin();
    while ( it != filters_.end() )
        it = filters_.erase(it);
}

void CoalaModel::saveRecordData()
{
  string path, basePath = loop_.workingDirectory() + "../data/out/";
  mkdir( basePath.c_str(), 755 );
  if ( buffer1_ )
  {
      printf( "Writing ADC data to data/out/adc.txt...\n");
      path = basePath + "adc.txt";
      cout << path << endl;
      FILE* out1 = fopen( path.c_str(), "w" );
      for ( long s = 0; s < recordLimit_; ++s )
      {
          fprintf( out1, "%e\n", buffer1_[s] );
      }
      fclose( out1 );
      //((CoalaWrapper&)loop_).modalToolbox_->outputFFT( buffer1_, recordLimit_ );
      delete buffer1_;
  }
  if ( buffer2_ )
  {
      printf( "Writing DAC data to data/out/dac.txt...\n");
      path = basePath + "dac.txt";
      FILE* out2 = fopen( path.c_str(), "w" );
      for ( long s = 0; s < recordLimit_; ++s )
      {
          fprintf( out2, "%e\n", buffer2_[s] );
      }
      fclose( out2 );
      delete buffer2_;
  }
  if ( buffer3_ )
  {
      printf( "Writing command data to data/out/command.txt...\n");
      path = basePath + "command.txt";
      FILE* out3 = fopen( path.c_str(), "w" );
      for ( long s = 0; s < recordLimit_; ++s )
      {
          fprintf( out3, "%e\n", buffer3_[s] );
      }
      fclose( out3 );
      delete buffer3_;
  }
  if ( buffer4_ )
  {
      printf( "Writing excitation data to data/out/excitation.txt...\n");
      path = basePath + "excitation.txt";
      FILE* out4 = fopen( path.c_str(), "w" );
      for ( long s = 0; s < recordLimit_; ++s )
      {
          fprintf( out4, "%e\n", buffer4_[s] );
      }
      fclose( out4 );
      delete buffer4_;
  }
  if ( loopTiming_ )
  {
      printf( "Writing loop timing data to data/out/looptiming.txt...\n");
      path = basePath + "looptiming.txt";
      FILE* timing = fopen( path.c_str(), "w" );
      for ( long s = 5; s < recordLimit_; ++s )
      {
          fprintf( timing, "%ld\n", (long) loopTiming_[s]/100 );
      }
      fclose( timing );
      delete loopTiming_;
  }
  if ( time_ )
  {
      printf( "Writing time data to data/out/time.txt...\n");
      path = basePath + "time.txt";
      FILE* time = fopen( path.c_str(), "w" );
      for ( long s = 0; s < recordLimit_; ++s )
      {
          fprintf( time, "%e\n", (float) time_[s]*1.0e-9 );
      }
      fclose( time );
      delete time_;
  }
  path = basePath + "parameters.txt";
  FILE* params = fopen ( path.c_str(), "w" );
  fprintf( params, "%e\n%f\n%f", loop_.getSamplingTime(), loop_.getTimeLimit(), gain_ );
  fclose( params );
  printf( "(finished writing data)\n");
}

void CoalaModel::cleanupAfterRun()
{
    if ( recordEnabled() && !loop_.wasStopped() )
      saveRecordData();
}

bool CoalaModel::initialize( bool /* testMode */ )
{
    printf ("CoalaModel::initialize\n" );

    if ( recordOn_ )
    {
        printf ("\tData record on\n" );
        recordLimit_ = (long) fmin( MAX_SAMPLE_BUFFER, (long) ( (float) loop_.getTimeLimit() / loop_.getSamplingTime() ) );
        printf ("%d samples to be recorded\n", recordLimit_ );
        buffer1_ = new float[recordLimit_];
        buffer2_ = new float[recordLimit_];
        buffer3_ = new float[recordLimit_];
        buffer4_ = new float[recordLimit_];
        loopTiming_ = new long long[recordLimit_];
        time_    = new long long[recordLimit_];
    }

    for ( int i = 0; i < filters_.size(); ++i )
    {
        if ( filters_[i]->enabled() )
            filters_[i]->initialize();
    }
}

void CoalaModel::startup()
{
    //TODO
}

void CoalaModel::prepareForStartup()
{
    y_ = 0;
    u_ = 0;
    tBefore_ = realtimeManager_.getHighPrecisionNanosecondTime();
}

void CoalaModel::step()
{
    t_ = realtimeManager_.getHighPrecisionNanosecondTime();
    k_ = loop_.getStep();

    u_ = 0.0;
    float excitation = 0.0;
    float command = 0.0;

    for ( int i = 0; i < filters_.size() ; ++i )
    {
        if ( filters_[i]->enabled() )
        {
            float value = filters_[i]->step( y_, t_ );
            u_ += value; //filters_[i]->step( y_, t_ );
            if ( filters_[i]->hasSelfExcitation() )
                excitation += value;
            if ( filters_[i]->hasCommand() )
                command += value;
        }
    }
    u_ *= gain_;

    if ( recordOn_ && k_ < recordLimit_ )
    {
        buffer1_[k_] = y_;
        buffer2_[k_] = u_;
        buffer3_[k_] = command;
        buffer4_[k_] = excitation;
        time_[k_] = t_-loop_.getStartTime();
        loopTiming_[k_] = t_ - tBefore_; //in microseconds
    }
    tBefore_ = t_;
}

void CoalaModel::setCurrentInput( unsigned int /* index */, float value )
{
    y_ = value;
}

float CoalaModel::getCurrentOutput( unsigned int /* index */ ) const
{
    return u_;
}

bool CoalaModel::handleMessage( const string& message )
{
    size_t pos = message.find( "/coala/set/outputgain/" );
    if ( pos == 0 )
    {
        string value = message.substr( 22 );
        setGain( atof( value.c_str()) );
        return true;
    }
    pos = message.find( "/coala/set/record/enable/" );
    if ( pos == 0 )
    {
        string value = message.substr( 25 );
        recordOn_ = (bool ) atoi( value.c_str());
        return true;
    }

    // FIXME gen filter MUST BE THE LAST
    /* Carmine hack me */
    /*
    pos = message.find( "/coala/gen/recompile");
    if ( pos == 0 )
    {
        ((GenFilter *)filters_[filters_.size()-1])->unloadGenLibrary();
        system( "rm -f /usr/lib/coala/modules/libgenmodule.so" );
       //Attempt to clean library cache...
       //dlclose(genLibrary_);

         system( "make --directory=/usr/src/coala/gen -f /usr/src/coala/gen/Makefile clean && make --directory=/usr/src/coala/gen -f /usr/src/coala/gen/Makefile");
         cout<<"waiting"<<getchar();
         delete filters_[filters_.size()-1];
         filters_[filters_.size()-1] = new GenFilter (loop_);
        return true;
    }
    */
    for ( int i = 0; i < filters_.size(); ++i )
    {
        if ( filters_[i]->handleMessage( message ) )
            return true;
    }
    return false;
}


void CoalaModel::serializeStatus( stringstream& stream ) const
{
    for ( int i = 0; i < filters_.size(); ++i )
    {
        if ( filters_[i]->enabled() )
            filters_[i]->serializeStatus( stream );
    }
}
