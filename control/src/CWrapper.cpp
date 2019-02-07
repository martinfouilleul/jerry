#include "TomControl.h"
#include <iostream>

static Tom     model_tom;
static Control model_control;

extern "C"
{

void init(int sr, float om_des, float zeta_des, float gamma,
          float zeta, float omega, float l1, float l2)
{
    model_tom = Tom(sr, om_des, zeta_des);
    model_control = Control(&model_tom, gamma, zeta, omega, l1, l2);
}

float getNextU(float new_sample)
{
    float result = model_control.next(new_sample);
    return result;
}

}
