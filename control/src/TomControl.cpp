#include "TomControl.h"

#define WARMUP_N 1000
#define Square(a) ((a)*(a))

TomController::TomController(int sr,
			     float omega,
			     float zeta,
			     float gamma,
			     float omegaDesired,
			     float zetaDesired,
			     float l1,
			     float l2): m_sr(sr),
					m_omega(omega),
					m_zeta(zeta),
					m_gamma(gamma),
					m_omegaDesired(omegaDesired),
					m_zetaDesired(zetaDesired),
					m_l1(l1),
					m_l2(l2),
					m_dt(1/float(sr)),
					m_state(),
					m_mes(),
					warmup(0)
{}

float TomController::nextStep(float measure)
{
    m_mes[0]   = m_mes[1];
    m_mes[1]   = m_mes[2];
    m_mes[2]   = measure;

    m_state[0] = Square(m_dt) * (-2 * m_zetaDesired * m_omegaDesired / m_dt *
                                  (m_mes[1] - m_mes[0]) - Square(m_omegaDesired) * m_mes[1]) -
                                  m_mes[0] + 2 * m_mes[1];

    m_state[1] = Square(m_dt) * (-2 * m_zetaDesired * m_omegaDesired / m_dt *
                            (m_state[0] - m_mes[1]) - Square(m_omegaDesired) * m_state[0]) -
                            m_mes[1] + 2 * m_state[0];

    float u     =  1 / m_gamma *
                    (m_sr*m_sr * (m_state[1] + m_mes[1] - 2*m_state[0])
                    + dfdt(m_mes, 3) * (2 * m_zeta * m_omega - m_l1)
                    + measure * (Square(m_omega) - m_l2)
                    + m_l1 * m_sr * (m_state[0] - m_mes[1])
                    + m_l2 * m_state[0]);

    return warmup > WARMUP_N ? u : u*(float(warmup++)/float(WARMUP_N));
}

float TomController::dfdt(float *x, int N)
{
    return (x[N-1] - x[N-2])/m_dt;
}
