#include "TomControl.h"
#include <iostream>

static Tom     model_tom;
static Control model_control;

extern "C"
{

void init()
{
    model_tom = Tom(44100/*sr*/,
                    300/*omega_des*/,
                    .02/*zeta_des*/);
    model_control = Control(&model_tom,
                            1/*gamma*/,
                            .02/*zeta*/,
                            300/*omega*/,
                            .1/*l1*/,
                            .1/*l2*/);
    std::cout << "Init done" << std::endl;
}

float getNextU(float new_sample)
{
    float result = model_control.next(new_sample);
    std::cout << result << std::endl;
    return result;
}

}
