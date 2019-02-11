//Author: Antoine CAILLON & Martin FOUILLEUL
#pragma once

struct TomController
{
    TomController(int sr,
		  float omega,
		  float zeta,
		  float gamma,
		  float omegaDesired,
		  float zetaDesired,
		  float l1,
		  float l2);

    float nextStep(float measure);
    float dfdt(float *x, int N);

    float m_sr;
    float m_dt;

    float m_omegaDesired;
    float m_zetaDesired;

    float m_gamma;
    float m_zeta;
    float m_omega;
    float m_l1;
    float m_l2;

    float m_state[2];
    float m_mes[3];

    int warmup;
};
