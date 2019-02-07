import ctypes as c
import os

os.system("cd build && make")

ltc = c.cdll.LoadLibrary("build/libtomcontrol.so")

ltc.getNextU.argtypes = c.c_float,
ltc.getNextU.restype  = c.c_float

ltc.init()

getNextU = lambda f: ltc.getNextU(c.c_float(f))

print(getNextU(3))
