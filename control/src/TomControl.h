//Author: Antoine CAILLON & Martin FOUILLEUL
#pragma once


/**
 * This struct contains the modal parameters of the real membrane, and the
 * desired one (i.e frequency, damping). The nextStep function is assumed to
 * estimate the position, velocity and acceleration of the desired membrane
 * given the measurement of the two previous positions. The control variable u
 * is then calculated according to the actual and desired positions, speeds,
 * and accelerations. Parameters l1 and l2 are responsible for the behavior of
 * the PID.
*/
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
