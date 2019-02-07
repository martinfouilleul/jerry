#include "TomControl.h"

//#########################TOM METHODS IMPLEMENTATION###########################

Tom::Tom(int sr, float omega, float zeta): m_sr(sr), m_omega(omega), m_zeta(zeta),
m_dt(1/float(sr)), m_state(), m_mes()
{

}


float Tom::next(float new_sample)
{
    m_state[0] = m_state[1];
    m_state[1] = m_state[2];
    m_mes[0]   = m_mes[1];
    m_mes[1]   = new_sample;

    m_state[2] = m_dt * m_dt * (-2 * m_zeta * m_omega / m_dt *
                                  (m_mes[1] - m_mes[0]) - (m_omega * m_omega) * m_mes[1]) -
                                  m_mes[0] + 2 * m_mes[1];

    return m_state[2];

}


//######################CONTROL METHODS IMPLEMENTATION##########################

Control::Control(Tom *tom, float gamma, float zeta, float omega, float l1, float l2):
m_tom(tom), m_gamma(gamma), m_zeta(zeta), m_omega(omega), m_l1(l1), m_l2(l2)
{

}

float Control::next(float new_sample)
{
    float eta_d = m_tom->next(new_sample);
    float u     = 1./m_gamma*(df2dt2(m_tom->m_state, 3) + dfdt(m_tom->m_mes, 2) *
                (2 * m_zeta * m_omega - m_l1) + new_sample*(m_omega*m_omega - m_l2) +
                 m_l1 * dfdt(m_tom->m_state, 3) + m_l2 * eta_d);
    return u;
}

float Control::dfdt(float *x, int N)
{
    return (x[N-1] - x[N-2])/m_dt;
}

float Control::df2dt2(float *x, int N)
{
    return (x[N-1] + x[N-3] - 2*x[N-2])/(m_dt*m_dt);
}
