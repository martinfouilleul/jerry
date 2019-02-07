#pragma once

class Tom
{
public:
    Tom();
    Tom(int sr, float omega, float zeta);
    float next(float new_sample);
protected:
    int m_sr;
    float m_dt, m_omega, m_zeta, m_state[3], m_mes[2];
    friend class Control;
};

class Control
{
public:
    Control();
    Control(Tom *tom, float gamma, float zeta, float omega, float l1, float l2);
    float next(float new_sample);
    float dfdt(float *x, int N);
    float df2dt2(float *x, int N);
protected:
    Tom *m_tom;
    float m_sr, m_dt, m_gamma, m_zeta, m_omega, m_l1, m_l2;

};
