import numpy as np
import matplotlib.pyplot as plt

class Tom(object):
    """Class describing a dynamic system modeled with 1 mode"""
    def __init__(self, sr=44100, omega=2*np.pi*218, zeta=2e-2):
        self.sr    = sr
        self.dt    = 1/sr
        self.omega = omega
        self.zeta  = zeta

        self.state = np.zeros([3])
        self.mes   = np.zeros([2])

    def next(self, new_sample):
        self.state = np.roll(self.state, -1)
        self.mes   = np.roll(self.mes, -1)

        self.mes[-1] = new_sample

        self.state[-1] = self.dt**2 * (-2 * self.zeta * self.omega / self.dt * \
                            (self.mes[1] - self.mes[0]) - self.omega**2 * self.mes[1])-\
                            self.mes[0] + 2 * self.mes[1]
        return self.state[-1]

if __name__ == "__main__":
    tom = Tom()
    tom.mes[-1] = 1
    x = np.zeros(44100)
    x[0] = 1
    for i in range(1,44100):
        x[i] = tom.next(x[i-1])
    plt.plot(x)
    plt.show()
