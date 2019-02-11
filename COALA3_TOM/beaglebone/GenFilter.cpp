//
//      .-. .-.   .-. .-. . . .-. .-. .-. .   
//      |(   |    |   | | |\|  |  |(  | | |   
//      ' '  '    `-' `-' ' `  '  ' ' `-' `-' 
//
//      (C) Ircam 2015
//      GenFilter.cpp
//      programmed by Robert Piechaud
//

#include "GenFilter.h"
#include "SignalConverter.h"
#include "gen/GenWrapper.h"
#include <iostream>
#include <string.h>
#include <cstdlib>
#include <dlfcn.h>
#include <math.h>
#include <vector>
#include <unistd.h>

using namespace std;

GenFilter::GenFilter( ControlLoopFacade& loop ):
    FilterInterface  ( loop ),
    genLibrary_      ( NULL ),
    compiling_       ( false ),
    genWrapperCreateFun_  ( NULL ),
    genWrapperDestroyFun_ ( NULL ),
    genWrapperResetFun_   ( NULL ),
    genWrapperPerformFun_ ( NULL ),
    genWrapperSetParameterFun_ ( NULL )
{
    cout << "genFilter instanciation " << endl;
    loadGenLibrary();
    reset();
}

GenFilter::~GenFilter()
{
    if ( genWrapperDestroyFun_ )
        (*genWrapperDestroyFun_)();
    unloadGenLibrary();
}

bool GenFilter::initialize()
{
    //if ( genWrapperResetFun_ )
    //    (*genWrapperResetFun_)( loop_.getSamplingTime() );
    cout << "gen: initialized before run" << endl;
}

void GenFilter::reset()
{
    if ( genWrapperResetFun_ )
        (*genWrapperResetFun_)( loop_.getSamplingTime() );
    cout << "gen: reset" << endl;
}

bool GenFilter::loadGenLibrary()
{
    std::string url = "/usr/lib/coala/modules/";
    url += getModuleName();
    cout << "loading module " << url << "..." << endl;
    genLibrary_ = dlopen(url.c_str(), RTLD_NOW);
    if ( genLibrary_ )
    {
        cout << "gen module loaded! :-) ?" << genLibrary_<< endl;
        if ( !instanciateLibFunction( "Create", (void**) &genWrapperCreateFun_ ) )
            return false;
        (*genWrapperCreateFun_)( loop_.getSamplingTime() );
        if ( !instanciateLibFunction( "Destroy", (void**) &genWrapperDestroyFun_ ) )
            return false;
        if ( !instanciateLibFunction( "Reset", (void**) &genWrapperResetFun_ ) )
            return false;
        if ( !instanciateLibFunction( "Perform", (void**) &genWrapperPerformFun_ ) )
            return false;
        if ( !instanciateLibFunction( "SetParameter", (void**) &genWrapperSetParameterFun_ ) )
            return false;
        //reset();
        return true;
    }
    const char* dlsym_error = dlerror();
    cout << "oops... could not load gen module! " << dlsym_error << endl;
    return false;
}
bool GenFilter::loadGenLibraryPlug(const char* name)
{
    std::stringstream ss;
    ss << "/usr/lib/coala/modules/libgenmodule_" << name <<".so";
    std::string lib_name = ss.str();
    const char *cstr = lib_name.c_str();
    genLibrary_ = dlopen(cstr, RTLD_NOW);
    if ( genLibrary_ )
    {
        cout << "plug module loaded! :-) ?" << name << genLibrary_<< endl;
        if ( !instanciateLibFunction( "Create", (void**) &genWrapperCreateFun_ ) )
            return false;
        (*genWrapperCreateFun_)( loop_.getSamplingTime() );
        if ( !instanciateLibFunction( "Destroy", (void**) &genWrapperDestroyFun_ ) )
            return false;
        if ( !instanciateLibFunction( "Reset", (void**) &genWrapperResetFun_ ) )
            return false;
        if ( !instanciateLibFunction( "Perform", (void**) &genWrapperPerformFun_ ) )
            return false;
        if ( !instanciateLibFunction( "SetParameter", (void**) &genWrapperSetParameterFun_ ) )
            return false;
        return true;
    }
    const char* dlsym_error = dlerror();
    cout << "oops... could not load gen module! " << dlsym_error << endl;
    return false;
}

void GenFilter::unloadGenLibrary()
{
    if ( genLibrary_ )
    {
        (*genWrapperDestroyFun_)();
        if ( dlclose( genLibrary_ ) == 0 )
        {
            cout << "gen module unloaded..." << endl;
            genLibrary_ = NULL;
            genWrapperCreateFun_ = NULL;
            genWrapperDestroyFun_ = NULL;
            genWrapperResetFun_ = NULL;
            genWrapperPerformFun_ = NULL;
            genWrapperSetParameterFun_ = NULL;
        }
        else
            cout <<"problem unloading";
    }
}

void GenFilter::setParameter( const char* name, double value )
{
    if ( !genWrapperSetParameterFun_ )
        return;
    cout << "gen: setting parameter " << name << " to " << value << endl;
    (*genWrapperSetParameterFun_)( name, value );
}

bool GenFilter::instanciateLibFunction( const char* name, void** fun )
{
    if ( !genLibrary_ )
        return false;
    *(void **) (fun) = dlsym( genLibrary_, name );
    const char* dlsym_error = dlerror();
    if ( dlsym_error )
    {
        cout << "oops... function " << name << " could not be found!" << dlsym_error << endl;
        unloadGenLibrary();
        return false;
    }
    cout << "genlib: function " << name << " loaded :-)" << endl;
    return true;
}

const char* GenFilter::getModuleName()
{
    static char name [128];
    FILE* file = fopen("/usr/lib/coala/modules/libgenname.txt", "rb");
    fseek(file,0,SEEK_END);
    size_t size = ftell(file);
    rewind(file);
    char*uuid = new char[size];
    fread(uuid, 1, size, file);
    uuid[size-1] = 0;
    fclose( file );
    sprintf(name, "libgenmodule%s.so", uuid);
    delete[] uuid;
    return name;
}

void GenFilter::recompileGenModule()
{
    bool isBypassed = loop_.isBypassed();
    loop_.bypass( true );    
    compiling_ = true;
    cout << "recompiling gen module..." << endl;
    unloadGenLibrary();
    system( "rm -f /usr/lib/coala/modules/libgenmodule*" );
    //Attempt to clean library cache...	 
    //dlclose(genLibrary_);
    system("date +%s%N > /usr/lib/coala/modules/libgenname.txt");
    FILE* file = fopen("/usr/lib/coala/modules/libgenname.txt", "rb");
    fseek(file,0,SEEK_END);
    size_t size = ftell(file);
    rewind(file);
    char* uuid = new char[size+1];
    fread(uuid, 1, size, file);
    fclose( file );
    uuid[size-1] = 0;
    cout << "uuid: " << uuid << " " << endl;
    system( "make --directory=/usr/src/coala/gen -f /usr/src/coala/gen/Makefile clean && make --directory=/usr/src/coala/gen -f /usr/src/coala/gen/Makefile");
    char cmd [256];
    sprintf( cmd, "mv /usr/lib/coala/modules/libgenmodule.so /usr/lib/coala/modules/libgenmodule%s.so", uuid );
    cout << "executing: " << cmd << endl;
    system( cmd );
    delete[] uuid;
    //cout << "waiting..." << getchar();
    if(!loadGenLibrary())
        cout <<"ERROR LOAD GEN"<< endl;
    //initialize();
    compiling_ = false;
    loop_.bypass( isBypassed );
}

void GenFilter::resetForStartup()
{
    //initialize();
}

float GenFilter::step( float input, long long t ) // t in nanosec.
{
    if ( !genWrapperPerformFun_ || compiling_ )
        return 0.;
    long long t0 = loop_.getStartTime();
    float output = 0.;
    output = (*genWrapperPerformFun_)( input );
    //cout << "input: " << input << " output: " << output << endl;
    return output;
}

bool GenFilter::handleMessage( const string& message )
{
    size_t pos = message.find( "/coala/gen/enable/" );
    if ( pos == 0 )
    {
        string value = message.substr( 18 );
        enable( (bool ) atoi( value.c_str()) );
        return true;
    }
    pos = message.find( "/?genon=" );
    if ( pos == 0 )
    {
        string value = message.substr( 8 );
        enable( (bool ) atoi( value.c_str()) );
        return true;
    }
    if ( message.find( "/coala/gen/recompile" ) == 0 )
    {
        recompileGenModule();
        return true;
    }
    if ( message.find( "/coala/gen/reload" ) == 0 )
    {
        unloadGenLibrary();
        loadGenLibrary();
        return true;
    }
    if ( message.find( "/coala/gen/reset" ) == 0 )
    {
        reset();
        return true;
    }
    if ( message.find( "/coala/gen/unload" ) == 0 )
    {
        unloadGenLibrary();
        return true;
    }
    if ( message.find( "/coala/gen/plug/name/" ) == 0)
    {
        string value = message.substr( 21 );
        cout << "current lib renamed " << ( value.c_str())<<endl;
        std::stringstream ss;
        ss << "mv /usr/lib/coala/modules/libgenmodule.so /usr/lib/coala/modules/libgenmodule_" << value << ".so";
        std::string s = ss.str();
        const char *command = s.c_str();
        system( (const char *)s.c_str());
        return true;
    }
    if ( message.find( "/coala/gen/plug/load/" ) == 0)
    {
        string value = message.substr( 21 );
        cout << "loading " << ( value.c_str())<<endl;
        loadGenLibraryPlug(value.c_str());
        return true;
    }

    pos = message.find( "/coala/gen/set/" );
    if ( pos == 0 )
    {
        char name [32];
        double value = 0.;
        const char* p1 = strchr( message.c_str()+pos+15, '/' );
        if ( p1 )
        {
            int l = p1 - (message.c_str()+pos+15);
            strncpy( name, message.c_str()+pos+15, l );
            name[l] = 0;
            int n = sscanf( p1+1, "%lf", &value );
            if ( n > 0 )
                setParameter( name, value );
            return true;
        }
    }
    //TODO: handle any possible parameter (float values)
    return false;
}

void GenFilter::serializeStatus( std::stringstream& stream ) const
{
    stream << "gen=" << (enabled()?"true":"false") << "+";
    //TODO: view parameters (introspection ?..)
}
