//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      (C) Ircam 2014
//      ModalFilter.ccp
//      programmed by Robert Piechaud
//

#include "ModalFilter.h"
#include "ModalToolbox.h"
#include "CoalaWrapper.h"
#include "maths/SquareMatrix.h"
#include "maths/Vector.h"
#include "core/RealTimeInterface.h"
#include "core/ControlLoopFacade.h"
#include <stdlib.h>
#include <rtdk.h>
#include <unistd.h>
#include <math.h>
#include <linux/input.h>
#include <fcntl.h>

#define MAX_SAMPLE_BUFFER 5000000

using namespace std;

ModalFilter::ModalFilter( ControlLoopFacade& controlLoop ):
    FilterInterface	 ( controlLoop ),
    numModes_        ( 0 ),
    loaded_          ( false ),
    Y_               ( 0 )
{
    //NOTHING
}

ModalFilter::~ModalFilter()
{
    //NOTHING
}

bool ModalFilter::loadContinuousDataFromMatlabFiles()
{   
    if ( !enabled() )
        return true;
    printf( "Loading data from Matlab...\n");
    
    K_.loadFromFile( loop_.workingDirectory() + "../data/in/Kc.txt" );
    C_.loadFromFile( loop_.workingDirectory() + "../data/in/Cc.txt" );
    
    if ( true ) //!loaded_
    {
        if ( !A_.loadFromFile( loop_.workingDirectory() + "../data/in/Ac.txt" ) ||
             !B_.loadFromFile( loop_.workingDirectory() + "../data/in/Bc.txt" ) ||
             !L_.loadFromFile( loop_.workingDirectory() + "../data/in/Lc.txt" ) )
        {
            printf( "(Model error: error when loading matlab entry data! :-(\n" );
            return false;
        }
        int numModes = B_.dimension()/2;
        numModes_ = numModes;
        eLd_.resize( 2*numModes );
        X_.resize( 2*numModes );
        X_next_.resize( 2*numModes );
        adjustVectorsOrientation();
        discretize();
        loaded_ = true;
    }
    else
    {
        Ad_.loadFromFile( loop_.workingDirectory() + "../data/out/Ad.txt" );
        Bd_.loadFromFile( loop_.workingDirectory() + "../data/out/Bd.txt" );
        Ld_.loadFromFile( loop_.workingDirectory() + "../data/out/Ad.txt" );
        int numModes = Bd_.dimension()/2;
        numModes_ = numModes;
        eLd_.resize( 2*numModes );
        X_.resize( 2*numModes );
        X_next_.resize( 2*numModes );
        adjustVectorsOrientation();
        printf( "(Model: discretization previously performed :-)\n" );
    }
    return true;
}

void ModalFilter::adjustVectorsOrientation()
{
    B_.setIsColumn();
    Bd_.setIsColumn();
    L_.setIsColumn();
    Ld_.setIsColumn();
    //Ld_.setIsColumn();
    eLd_.setIsColumn();
    X_.setIsColumn();
    X_next_.setIsColumn();
}

void ModalFilter::unload()
{
    numModes_ = 0;
    A_.reset();
    Ad_.reset();
    B_.reset();
    Bd_.reset();
    C_.reset();
    L_.reset();
    Ld_.reset();
    K_.reset();
    Y_ = 0;
}

bool ModalFilter::initialize()
{
    printf ( "ModalFilter: initialization\n" );
    return loadContinuousDataFromMatlabFiles();
}

void ModalFilter::startup()
{
    //TODO
}

void ModalFilter::resetForStartup()
{
    Y_ = 0;
    X_.zero();
    X_next_.zero();
}

float ModalFilter::step( float input, long long /* t */ )
{
    // e = y - y~ :
    float e = input - Y_;

    // eL = e*L :
    eLd_ = Ld_;
    eLd_ *= e;

    // Xk+1 = e*L + uk*Bd + Ad*Xk
    X_next_.multiply( &Ad_, &X_ );   // Xk+1 = Ad*Xk
    X_next_.add( &eLd_ );            // Xk+1 += e*L 
    X_next_.add( &Bd_, input );         // Xk+1 += e*
    
    // preparing the future...
    X_ = X_next_;
    float output = -(K_*X_);
    Y_ = C_*X_;
    return output;
}

void ModalFilter::discretize()
{
    double T = loop_.getSamplingTime();
    printf("Model: zero order hold discretization (%d modes, sampling time = %.2f µs)...\n", numModes_, (float) T*1.e6);
    long long t0 = loop_.getHighPrecisionNanosecondTime();

    //cout << "L:\n" << L_;
    int dim = 2*numModes_;
    Ad_.resize( dim );
    Bd_.resize( dim );
    Ld_.resize( dim );
    
    SquareMatrix A( A_ );
    SquareMatrix Id( dim );
    Id.identity();
 
    // pour le system observateur avec Ao = (A-LC) etant la nouvelle matrice de system de l'observateur:
    // Aod = e^(Ao * Ts)
    // Ld = Ao^(-1) * (Aod - I) * L
    
    SquareMatrix LC (Id);
    LC.vectorMultiplication( L_, C_ );
    SquareMatrix A0 = A -LC;
    //cout << L_;
    //cout << A0;
    SquareMatrix expA0T = exp( A0*T );
    //cout << expA0T;
    SquareMatrix A0inv= A0^-1;
    if (A0inv.isNull())
    {
       Ld_.loadFromFile(  loop_.workingDirectory() + "../data/in/Ld.txt" );
       printf( "ModalFilter: could not discretize vector L!\n" );
    }
    else
       Ld_ = (A0^-1)*(expA0T-Id)*L_;
    SquareMatrix expAT = exp( A*T );
    Ad_ = expAT;
    Bd_ = (A^-1)*(expAT-Id)*B_;
    SquareMatrix M (Ad_);
    M.saveToFile( loop_.workingDirectory() + "../data/out/Ad.txt" );
    Ld_.saveToFile( loop_.workingDirectory() + "../data/out/Ld.txt" );
    Bd_.saveToFile( loop_.workingDirectory() + "../data/out/Bd.txt" );
    long long t1 = loop_.getHighPrecisionNanosecondTime();

    printf("\t(discretization computed in %.2f ms)\n", (float)(t1-t0)*1.0e-6 );
}

bool ModalFilter::handleMessage( const string& message )
{
    size_t pos = message.find( "/coala/set/modalcontrol/enable/" );
    if ( pos == 0 )
    {
        string value = message.substr( 31 );
        enable( (bool ) atoi( value.c_str()) );
        return true;
    }
    pos = message.find( "/?activecontrolon=" );
    if ( pos == 0 )
    {
        string value = message.substr( 18 );
        enable( (bool ) atoi( value.c_str()) );
        return true;
    }
    return false;
}

void ModalFilter::serializeStatus( std::stringstream& stream ) const
{
    stream << "modal_control=" << (enabled()?"true":"false") << "+modes=" << getModesAmount() << "+";
}

