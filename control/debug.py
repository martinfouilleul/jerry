import os
import ctypes as c
import numpy as np
import matplotlib.pyplot as plt

from scipy.signal import firwin, convolve

os.system("cd build && make")

ltc = c.cdll.LoadLibrary("build/libtomcontrol.so")

ltc.getNextU.argtypes = c.c_float,
ltc.getNextU.restype  = c.c_float
ltc.init.argtypes = c.c_int, c.c_float, c.c_float, c.c_float, c.c_float, c.c_float,\
                    c.c_float, c.c_float

getNextU = lambda f: ltc.getNextU(c.c_float(f))
init     = lambda sr, omdes, zetades, gamma, zeta, omega, l1, l2:\
            ltc.init(c.c_int(sr),\
                     c.c_float(omdes),\
                     c.c_float(zetades),\
                     c.c_float(gamma),\
                     c.c_float(zeta),\
                     c.c_float(omega),\
                     c.c_float(l1),\
                     c.c_float(l2))


sr = 22050


init(sr, np.pi*2*200, 2e-2, 100, 2e-2, np.pi*2*218, .1, .1)


x_c = np.zeros(sr)
t = np.linspace(0,1,sr)

disp = (np.sin(2*np.pi*218*t) + np.random.randn(sr)/30)*1e-3

disp = convolve(firwin(100, 300/sr), disp)

for i in range(sr):
    x_c[i] = getNextU(disp[i])

plt.plot(x_c)
plt.show()
