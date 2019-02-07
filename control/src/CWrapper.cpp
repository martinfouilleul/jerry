#include "TomControl.h"

static Tom     model_tom     = Tom(44100/*sr*/,
                                   300/*omega_des*/,
                                   .02/*zeta_des*/);
static Control model_control = Control(&model_tom,
                                       1/*gamma*/,
                                       .02/*zeta*/,
                                       300/*omega*/,
                                       .1/*l1*/,
                                       .1/*l2*/);

extern "C"
{

float getNextU(float new_sample)
{
    return model_control.next(new_sample);
}

}
