// EnvelopeFollower.h
// 

#ifndef ENVELOPEFOLLOWER_H
#define ENVELOPEFOLLOWER_H 

#include <cmath>

namespace nanosound {
	template <typename T>
	//! Follows amplitude envelope of a signal
	class EnvelopeFollower {
	public:
		EnvelopeFollower (double feedback = .999) {
			m_val = 0;
			m_feedback = feedback;
			m_gain = 1. - m_feedback;
		}
		virtual ~EnvelopeFollower () {}
		void reset () { m_val = 0; }
		T process (const T in) {
			T max = in;
			double abs = fabs (in);
			m_val = abs > m_val ? abs : m_val * m_feedback + abs * m_gain;
			if (max < m_val) max = m_val;
			return max;
		}
		void setMax (T m) { if (m > m_val) m_val = m; }
		void setFeedback (T f) {
			m_val = 0;
			m_feedback = f;
			m_gain = 1. - m_feedback;			
		}
	private:
		T m_val;
		double m_feedback;
		double m_gain;
	};
}
#endif	// ENVELOPEFOLLOWER_H 

// EOF
