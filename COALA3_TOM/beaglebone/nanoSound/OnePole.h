// OnePole.h
// 

#ifndef ONEPOLE_H
#define ONEPOLE_H 

#include "algorithms.h"
#include <cmath>

namespace nanoSound {
	template <typename T>
	class OnePole {
	private:
		OnePole& operator= (OnePole&);
		OnePole (const OnePole&);
	public: 
		OnePole (T sr, T cutoff, int order = 2, bool lp = true) : m_y1 (0) {
			m_st = 1. / sr;
			m_order = order;
			m_y1 = new T[m_order];
			m_lp = lp;
	
			this->cutoff (cutoff);
		}
		virtual ~OnePole () {
			delete [] m_y1;
		}
		void reset () {
			for (int j = 0; j <  m_order; ++j) {
				m_y1[j] = 0;
			}
		}
		void cutoff (T c) {
			T omega = TWOPI * c;	
			T b = 2. - cos (omega * m_st);
	
			if (m_lp) {
				m_a1 = cos (omega * m_st) - 2. + 
					sqrt ((b * b)  - 1.);
				m_b0 = 1. + (m_a1);
			} else {
				m_a1 = b - sqrt ((b * b)  - 1.);
				m_b0 = 1. - (m_a1);
			}
			reset ();
		}
		T* process (const T* input, T* output, int size) {
			T y = 0;
			T* tmp = (T*) input;
			for (int j = 0; j <  m_order; ++j) {
				for (int i = 0; i < size; ++i) {
					y = (m_b0 * tmp[i]) - (m_a1 * m_y1[j]);
					output[i] = m_y1[j] = y;
				}
				tmp = output;
			}		
			return output;
		}
	private:
		T m_st;
		T* m_y1;
		T m_a1;
		T m_b0;
		int m_order;
		bool m_lp;
	};
}
#endif	// ONEPOLE_H 

// EOF
