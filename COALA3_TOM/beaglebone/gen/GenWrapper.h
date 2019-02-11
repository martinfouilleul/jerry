#ifndef _GEN_MODULE_H_
#define _GEN_MODULE_H_

/*
#include "genlib.h"
#include "genlib_exportfunctions.h"
#include "genlib_ops.h"
*/

/*
class GenModuleWrapper
{
  public:

    GenModuleWrapper();
    virtual ~GenModuleWrapper();
    void setValue( float value );
    float  getValue();

private:
   float x_;
};

extern "C"
{
	GenModuleWrapper* createGenWrapper();
	void destroyGenWrapper(GenModuleWrapper* instance);
}
*/

extern "C" {
    
    //int perform(CommonState *cself, t_sample **ins, long numins, t_sample **outs, long numouts, long n);
    //void reset( CommonState *cself );
    
    //int num_inputs();
    //int num_outputs();
    //int num_params();
    //void setparameter(CommonState *cself, long index, t_param value, void *ref);
    //void getparameter(CommonState *cself, long index, t_param *value);
    //const char *getparametername(CommonState *cself, long index);
    //t_param getparametermin(CommonState *cself, long index);
    //t_param getparametermax(CommonState *cself, long index);
    //char getparameterhasminmax(CommonState *cself, long index);
    //const char *getparameterunits(CommonState *cself, long index);
    //size_t getstatesize(CommonState *cself);
    //short getstate(CommonState *cself, char *state);
    //short setstate(CommonState *cself, const char *state);
    
    void* Create( double sampleRate );
    void  Destroy();
    float Perform( float input );
    void  Reset( double sampleRate );
    void  SetParameter( const char* name, double value );
    void __attribute__ ((constructor)) my_init();
    void __attribute__ ((destructor)) my_fini();
    
}
 
#endif  // _GEN_MODULE_H_
