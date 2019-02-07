//AUTHOR: ANTOINE CAILLON
#include "TomControl.h"
#include <iostream>

#define WARMUP_N 1000

//#########################TOM METHODS IMPLEMENTATION###########################

Tom::Tom(int sr, float omega, float zeta): m_sr(sr), m_omega(omega), m_zeta(zeta),
m_dt(1/float(sr)), m_state(), m_mes()
{

}

Tom::Tom()
{

}

void Tom::next(float new_sample)
{
    m_state[0] = 0;
    m_state[1] = 0;
    m_mes[0]   = m_mes[1];
    m_mes[1]   = m_mes[2];
    m_mes[2]   = new_sample;

    m_state[0] = m_dt * m_dt * (-2 * m_zeta * m_omega / m_dt *
                                  (m_mes[1] - m_mes[0]) - (m_omega * m_omega) * m_mes[1]) -
                                  m_mes[0] + 2 * m_mes[1];

    m_state[1] = m_dt * m_dt * (-2 * m_zeta * m_omega / m_dt *
                            (m_state[0] - m_mes[1]) - (m_omega * m_omega) * m_state[0]) -
                            m_mes[1] + 2 * m_state[0];
}


//######################CONTROL METHODS IMPLEMENTATION##########################

Control::Control(Tom *tom, float gamma, float zeta, float omega, float l1, float l2):
m_tom(tom), m_gamma(gamma), m_zeta(zeta), m_omega(omega), m_l1(l1), m_l2(l2), warmup(0)
{
    m_sr = tom->m_sr;
    m_dt = tom->m_dt;
}

Control::Control()
{

}

float Control::next(float new_sample)
{
    m_tom->next(new_sample);
    float u     =  1 / m_gamma *
                    (m_sr*m_sr * (m_tom->m_state[1] + m_tom->m_mes[1] - 2*m_tom->m_state[0])
                    + dfdt(m_tom->m_mes, 3) * (2 * m_zeta * m_omega - m_l1)
                    + new_sample * (m_omega * m_omega - m_l2)
                    + m_l1 * m_sr * (m_tom->m_state[0] - m_tom->m_mes[1])
                    + m_l2 * m_tom->m_state[0]);

    return warmup > WARMUP_N ? u : u*(float(warmup++)/float(WARMUP_N));
}

float Control::dfdt(float *x, int N)
{
    return (x[N-1] - x[N-2])/m_dt;
}
