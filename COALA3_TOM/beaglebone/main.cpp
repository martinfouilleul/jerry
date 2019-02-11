//
//      .-. .-.   .-. .-. . . .-. .-. .-. .
//      |(   |    |   | | |\|  |  |(  | | |
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-'
//
//      @ Ircam 2014
//      main.ccp
//      created by Robert Piechaud
//

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "CoalaWrapper.h"
#include "maths/SquareMatrix.h"
#include "maths/VectorDouble.h"
#include "maths/Vector.h"

int main( int argc, char **argv )
{
    //char* wavFile = 0;
    float chirpGain = -999.;
    float gain = -999.;
    float timeMax = 0.;
    bool showHelp = false;
    bool realtimeMode = true;
    bool chirp = false;
    bool model = false;
    bool record = false;
    bool shareTime = true;
    bool verbose = false;
    int webserverPort = 0;
    int oscserverPort = 0;
    float samplePeriod = 0;
    float startFreq = 0;
    float endFreq = 0;
    char* matrix = 0;
    std::string workingDirectory ( "/root/dev/control_loop/projects/COALA2/bin/" );

    for (int i = 1; i < argc; ++i)
    {
      if ( strncmp(argv[i], "-realtime=", 10 ) == 0 )
      {
        if ( strcmp(argv[i]+10, "off" ) == 0 )
        {
          printf( "parameter passed: real time mode off\n" );
          realtimeMode = false;
        }
      }
      else if ( strncmp(argv[i], "-v", 2 ) == 0 )
      {
          printf( "parameter passed: verbose\n" );
          verbose = true;
      }
      else if ( strncmp(argv[i], "-sharetime", 10 ) == 0 )
      {
         printf( "parameter passed: shared time\n" );
         shareTime = true;
      }
      else if ( strncmp(argv[i], "-webserver", 10 ) == 0 )
      {
	        webserverPort = 10000;
          if ( argv[i][10] == '=' )
            webserverPort = atoi( argv[i] + 11 );
          printf( "parameter passed: web server (port %d)\n", webserverPort );
      }
      else if ( strncmp(argv[i], "-oscserver", 10 ) == 0 )
      {
	        oscserverPort = 8000;
          if ( argv[i][10] == '=' )
            oscserverPort = atoi( argv[i] + 11 );
          printf( "parameter passed: OSC server (port %d)\n", oscserverPort );
      }
      else if ( strncmp(argv[i], "-control=", 9 ) == 0 )
      {
        if ( strcmp(argv[i]+9, "on" ) == 0 )
        {
          printf( "parameter passed: active control on\n" );
          model = true;
        }
      }
      else if ( strncmp(argv[i], "-chirp=", 7 ) == 0 )
      {
        if ( strcmp(argv[i]+7, "on" ) == 0 )
        {
          printf( "parameter passed: chirp on\n" );
          chirp = true;
          record = true;
        }
      }
      else if ( strncmp(argv[i], "-record=", 8 ) == 0 )
      {
        if ( strcmp(argv[i]+8, "on" ) == 0 )
        {
          printf( "parameter passed: record mode on\n" );
          record = true;
        }
      }
      else if ( strncmp(argv[i], "-time=", 6 ) == 0 )
      {
        timeMax = atof( argv[i] + 6 );
        printf( "init: max time set to %f sec\n", timeMax );
      }
      else if ( strncmp(argv[i], "-gain=", 6 ) == 0 )
      {
        gain = atof( argv[i] + 6 );
        printf( "init: general gain set to %f\n", gain );
      }
      else if ( strncmp(argv[i], "-chirpGain=", 11 ) == 0 )
      {
        chirpGain = atof( argv[i] + 11 );
        printf( "init: chirp gain set to %f\n", chirpGain );
      }
      else if ( strncmp(argv[i], "-startFreq=", 11 ) == 0 )
      {
        startFreq = atof( argv[i] + 11 );
        printf( "init: chirp start freq. set to %f\n", startFreq );
        chirp = true;
        record = true;
      }
      else if ( strncmp(argv[i], "-endFreq=", 9 ) == 0 )
      {
        endFreq = atof( argv[i] + 9 );
        printf( "init: chirp end freq. set to %f\n", endFreq );
        chirp = true;
        record = true;
      }
      else if ( strncmp(argv[i], "-sampleFreq=", 12 ) == 0 )
      {
        float freq = atof( argv[i] + 12 );
        if ( freq > 0 )
        {
          printf( "parameter passed: sample frequency = %f Hz\n", freq );
          samplePeriod = 1/freq;
        }
      }
      else if ( strncmp(argv[i], "-samplePeriod=", 14 ) == 0 )
      {
        float microsec = atof( argv[i] + 14 );
        printf( "init: sample period set to %f μs\n", microsec );
        samplePeriod = microsec*1.0e-6;
      }
      else if ( strcmp(argv[i], "-help" ) == 0 || strcmp(argv[i], "-h" ) == 0 )
      {
        showHelp = true;
      }
      else if ( strncmp(argv[i], "-workingdirectory=", 18 ) == 0 )
      {
          workingDirectory = argv[i] + 18;
          if ( workingDirectory.length() )
          {
              if ( workingDirectory.at(workingDirectory.length() - 1) != '/' )
                  workingDirectory += "/";
                  std::cout << "parameter passed: working directory set to " << workingDirectory << std::endl;
          }
      }
      else if ( strncmp(argv[i], "-testMatrix=", 12 ) == 0 )
      {
        matrix = argv[i] + 14;
        printf( "test matrix stuff...\n" );

          // pour le system observateur avec Ao = (A-LC) etant la nouvelle matrice de system de l'observateur:
          // Aod = e^(Ao * Ts)
          // Ld = Ao^(-1) * (Aod - I) * L
      }
    }

    if ( !chirp && !model && !matrix && gain == -999. && !webserverPort && !oscserverPort )
    {
      printf( "\ninit error: no significant parameter was passed!\n" );
      showHelp = true;
    }
    if ( showHelp )
    {
      printf( "\n" );
      printf( "Active Control Loop - © Ircam 2016\n" );
      printf( "COALA v2: BeagleBone Black with custom cape\n\n" );
      printf( "Usage:\n" );
      printf( "  real time mode:      -realtime=[on/off] (on by default)\n" );
      printf( "  active control mode: -control=[on/off] (off by default)\n" );
      printf( "  chirp mode:          -chirp=[on/off] (off by default)\n" );
      printf( "  record mode:         -record=[on/off] (off by default except if chirp is on)\n" );
      printf( "  run time:            -time=[float value in seconds]\n" );
      printf( "  share time mode:     -sharetime\n" );
      printf( "  web server:          -webserver=[port]\n" );
      printf( "  OSC server:          -oscserver=[port]\n" );
      printf( "  sample period:       -samplePeriod=[float value in µs]\n" );
      printf( "  sample frequency:    -sampleFreq=[float value in Hz]\n" );
      printf( "  chirp start freq.:   -startFreq=[float value in Hz]\n" );
      printf( "  chirp end freq.:     -endFreq=[float value in Hz]\n" );
      printf( "  chirp gain:          -chirpGain=[float value] (0.3 by default)\n" );
      printf( "  working directory :  -workingdirectory=[path] (default is /root/dev/src/control_loop/projects/COALA2/\n" );
      printf( "  verbose:      -v\n" );
      //printf( "  test matrix :        -testMatrix=[matrix file name]\n" );
      return 0;
    }

    if ( matrix )
    {
        SquareMatrix M, P;// (2);
        //M.loadFromFile("analysis/M.txt");
        //P.loadFromFile("analysis/P1.txt");
        //std::cout << "M:\n" << M << "P:\n" << P << "P*M:\n" << P*M << "M*P:\n" << M*P;
        //return 0;
        //X.identity();
        //M *= 1.0e-6;
        //M.identity();

        SquareMatrix l, u, Pinv;
        SquareMatrix Id;
        SquareMatrix LU;
        Id.identity();

        /*
        M.loadFromFile("analysis/M3.txt");
        M.resize(100);
        M.randomize();
        M.saveToFile("analysis/Mx.txt");
        LU = M.LUdecomposition( P );
        Pinv = P;
        Pinv.transpose();
        LU.getLandUfromLU( l, u );
        std::cout << "M:\n" << M
                  << "LU:\n" << LU
                  << "P:\n" << P
                  << "P-1*L*U\n" << Pinv*l*u;
        SquareMatrix Minv = M^-1;
        std::cout << "M^-1:\n" << Minv << "M^-1*M:\n" << Minv*M;
        std::cout << "det(M) = " << M.determinant() << "\n";
        return 0;
        */

        SquareMatrix LC (Id);
        Vector C, L;
        M.loadFromFile("analysis/Ac.txt");
        C.loadFromFile("analysis/Cc.txt");
        L.loadFromFile("analysis/Lc.txt");
        L.setIsColumn();
        LC.vectorMultiplication( L, C );
        LC *= -1;

        /*
        M.loadFromFile("analysis/M.txt");
        LU = M.LUdecomposition( P );
        LU.getLandUfromLU( l, u );
        std::cout << "M:\n" << M << "LU:\n" << LU << "L:\n" << l << "U:\n" << u << "L*U\n" << l*u;
        return 0;
        */

        std::cout << "Ac:\n" << M << "LC:\n" << LC;

        SquareMatrix A0 ( M );
        A0 += LC;
        A0.saveToFile( "analysis/A0.txt" );
        VectorDouble diag = A0.getDiagonal();
        //std::cout << "diag(A0): " << diag;
        //M.resize(40);
        //M.randomize();
        std::cout << "A:\n" << M;
        //std::cout << "A0:\n" << A0;
        //std::cout << "A0:\n" << A0;
        //M.processIdealPivoting(P);
        //std::cout << "A (after pivot):\n" << M << "P:\n" << P << "P*A:\n" << P*M;
        LU = A0.LUdecomposition( P );
        Pinv = P;
        Pinv.transpose();
        LU.getLandUfromLU( l, u );
        std::cout << "A0:\n" << A0
                  << "LU:\n" << LU;
                  //<< "P-1*L*U\n" << Pinv*l*u;

        //M *=P;
        //X.loadFromFile("analysis/X.txt");
        //printf( "det(M): %f\n", M.determinant());


        //VectorDouble permutation;
        //SquareMatrix LU = M.simpleLU(), l, u;
        //std::cout << "LU:\n" << LU;
        //LU.getLandUfromLU( l, u );
        //std::cout << "L:\n" << l << "U:\n" << u << "L*U:\n" << l*u;
        //std::cout << "permutation:\n" << permutation;

        SquareMatrix A0inv = A0^-1;
        std::cout << "A0^-1:\n" << A0inv;

        std::cout << "(A0^-1)*A0:\n" << A0inv*A0;
        std::cout << "det(A0) = " << A0.determinant() << "\n";
        return 0;
        /*
        float T = 5.e-5;
        SquareMatrix A;
        Vector B, C, L;
        A.loadFromFile("analysis/Ac.txt");
        B.loadFromFile("analysis/Bc.txt");
        C.loadFromFile("analysis/Cc.txt");
        L.loadFromFile("analysis/Lc.txt");
        B.setIsColumn();
        L.setIsColumn();

        SquareMatrix Id( A.dimension() );
        Id.identity();
        SquareMatrix expAT = exp( A*T );
        SquareMatrix Ad (expAT);
        printf( "det(A): %f\n", A.determinant());
        return 0;
        */

        /*
        std::cout << "Ad:\n" << Ad;
        SquareMatrix Ainv;
        Ainv = A^-1;
        std::cout << "A^-1:\n" << Ainv;

        Vector Bd;
        Bd = Ainv*(Ad-Id)*B;
        std::cout << "Bd:\n" << Bd;
        */

        /*
        SquareMatrix LC (Id);
        Vector Ld;
        LC.vectorMultiplication( L, C );
        SquareMatrix A0 = A -LC;
        std::cout << "A0:\n" << A0;
        SquareMatrix expA0T = exp( A0*T );
        std::cout << "exp(A0*T):\n" << expA0T;
        //printf( "sup(A0) = %f\n", A0.sup() );
        //A0 = A0*0.001;
        A0.transpose();
        printf( "sup(A0) = %f\n", A0.sup() );
        printf( "det(A): %f\n", A.determinant());
        printf( "det(A0): %f\n", A0.determinant());
        SquareMatrix A0inv;
        A0inv = A0^-1;
        std::cout << "A0^-1:\n" << A0inv;
        Ld = A0inv*(expA0T-Id)*L;
        std::cout << "Ld:\n" << Ld;
        */
        return 0;
    }

    CoalaWrapper* coala = CoalaWrapper::getInstance( workingDirectory );
    coala->allowRealTime( realtimeMode );
    if ( timeMax > 0 )
    	coala->setTimeLimit( timeMax );

    if ( samplePeriod > 0 )
    {
    	coala->setSamplingTime( samplePeriod );
    }
    coala->setVerbose( verbose );
    coala->enableModel( model );
    coala->enableChirp( chirp );
    coala->enableRecord( record );
    coala->enableShareTime( shareTime );
    if ( webserverPort > 0 )
      coala->enableWebServer( webserverPort );
    if ( oscserverPort > 0 )
      coala->enableOSCServer( oscserverPort );
    if ( chirpGain > -999. )
      coala->setChirpGain( chirpGain );
    if ( gain > -999. )
      coala->setGain( gain );
    if ( startFreq > 0 || endFreq > 0 )
      coala->setChirpFrequencies( startFreq, endFreq );

    coala->launch();
    CoalaWrapper::deleteInstance();
    return 0;
}

/*
5-11-2014:

SIGDEBUG received, reason 3: triggered fault
./control_loop[0xb104]
/lib/libc.so.6(__default_rt_sa_restorer_v2+0x0)[0xb6ca00c0]
./control_loop[0xca6c]
./control_loop[0xd2d4]
./control_loop[0xd4c0]

*/
