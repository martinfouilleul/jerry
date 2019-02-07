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


sr = 44100



fm = 218
freq = [200, 210, 220, 230]



x_c = np.zeros(sr)
t = np.linspace(0,1,sr)

disp = (np.sin(2*np.pi*fm*t) + np.random.randn(sr)/30)*4e-3
disp*= np.exp(-2e-2*t*2*np.pi*fm)

# disp = np.pad(disp, (10000,0), "constant")

disp_filtered = convolve(firwin(100, 300/sr), disp)

plt.plot(disp/max(abs(disp)),".")

for f in freq:

    init(sr, np.pi*2*f, 2e-2, 100, 2e-2, np.pi*2*fm, .1, .1)

    for i in range(sr):
        x_c[i] = getNextU(disp_filtered[i])

    plt.plot(x_c)

plt.legend(["Membrane %dHz" % fm] + ["HP %dHz" % f for f in freq])
plt.grid()
plt.show()
