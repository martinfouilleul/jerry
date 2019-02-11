#include "GenWrapper.h"
#include "gen_exported.h"
#include "genlib.h"
#include "genlib_exportfunctions.h"
#include "genlib_ops.h"
#include <stdio.h>
#include <string.h>
#include <iostream>
 /*
extern "C"
{

GenModuleWrapper* createGenWrapper()
{
	return new GenModuleWrapper();
}

void destroyGenWrapper( GenModuleWrapper* instance )
{
	delete instance;
}

}

using namespace std;

GenModuleWrapper::GenModuleWrapper():
	x_ ( 0)
{
    cout << "GenModuleWrapper constructor!" << endl;
}

GenModuleWrapper::~GenModuleWrapper()
{
  //NOTHING
}

void GenModuleWrapper::setValue( float value )
{
	x_ = value;
	cout << "x: " << x_ << endl;
}

float  GenModuleWrapper::getValue()
{
	return x_;
}
*/

static CommonState* _self = NULL;


void my_init() { /* printf("Yo Genwrapper!\n"); */}
void __attribute__ ((destructor)) my_fini() { /* printf("Luss GenWrapper!\n"); */}

void* Create( double sampleRate )
{
    _self = (CommonState *) gen_exported::create( (t_param) 1/sampleRate, 1 );
    return (void*) _self;
}

void Destroy()
{
    if ( _self )
        gen_exported::destroy( _self );
}

t_sample* _output = new t_sample[16];
t_sample* _input = new t_sample[16];

float Perform( float finput )
{
    _input[0] = finput;
    
    if ( _self )
        gen_exported::perform( _self, &_input, (_self->numins > 0)? 1:0, &_output, 1, 1 );
    return (float) _output[0];
}

void Reset( double sampleRate )
{
    if ( _self )
    {
        _self->sr = 1/sampleRate;
        gen_exported::reset( _self );
    }
}

void SetParameter( const char* name, double value )
{
    int index = -1;
    for ( int i = 0; i < _self->numparams; ++i )
    {
        ParamInfo& param = _self->params[i];
        if ( strcmp( param.name, name ) == 0 )
        {
            index = i;
            break;
        }
    }
    if ( index == -1 )
        return;
    gen_exported::setparameter( _self, index, (t_param) value, NULL );
}
