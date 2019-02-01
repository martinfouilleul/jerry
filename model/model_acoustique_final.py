import numpy as np
from scipy.special import jv, jn_zeros
from scipy.integrate import quad

class Tom(object):
    """Object containing all parameters involved in the dfdt simulation of a tom

    Parameters
    ----------

    M: np.ndarray
        Modal mass matrix in diagonal form
    K: np.ndarray
        Modal stifness matrix in diagonal form
    C: np.ndarray
        Modal damping matrix in diagonal form
    fs: int
        Sampling rate
    Mm: float
        Mass of the HP's membrane
    t_1: float
        Flight time from HP to tom membrane
    rho: int
        Volumic mass of the air
    """
    def __init__(self, M, K, C, fs=44100, Mm=1, t_1=0, Beta=1):
        self.M = M
        self.K = K
        self.C = C
        self.fs = fs
        self.dt = 1/fs
        self.Beta = Beta
        self.Mm = Mm
        self.t_1 = t_1

        self.a = 16e-2
        self.V0 = np.pi*(33e-2/2)**2 * 23e-2
        self.Ca = 334
        self.rho = 1.225
        self.n_modes = M.shape[0]

        self.poly = np.load("poly.npy")

        self.dvdphi_n = 2*np.pi**2*np.array([quad(lambda r: jv(0,\
                        jn_zeros(0, n+1)[-1] * r / self.a), \
                        0, self.a)[0] for n in range(self.n_modes)])

        self.eta = np.zeros([n_modes, 3])

        self.exp_hp = np.zeros(2)



    def getNextDisplacement(self, x_hp) -> np.ndarray:
        """Yield the next membrane's displacement step
        """
        self.eta = np.roll(self.eta, -1, 1)
        self.eta[:,2] = (self.dt**2 / M).dot( \
            (self.Beta * self.Mm * dt2(x_hp) - self.C/self.dt * (self.eta[:,1] - self.eta[:,0]) \
            - self.K.dot(self.eta[:,1]))) + 2*self.eta[:,1] - self.eta[:,0]
        return self.eta[:,-1]

    def dt2(self, x) -> np.ndarray:
        acc = self.fs**2 * (x + self.exp_hp[0] - 2*self.exp_hp[1])
        self.exp_hp = np.roll(self.exp_hp, -1, 0)
        self.exp_hp[-1] = x
        return x


    def getMicroPressure(self, x):
        y = (self.rho * self.Ca**2 * x) / (self.V0 + self.dvdphi_n * self.eta[:,2])\
        + (self.rho * self.Ca**2 * self.eta[:,2] * self.dvdphi_n) / (self.V0 + x)

        return y


















#lolilol
