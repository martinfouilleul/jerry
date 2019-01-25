import torch
import numpy as np
from glob import glob
import librosa as li
import os

from scipy.linalg import hankel

def ESPRIT(s, ordre):
    r"""Straight forward implementation of ESPRIT algorithm

    ESPRIT stands for "Estimation of Signal Parameters via Rotationnal Invariance Techniques".

    Parameters
    ----------
    s: numpy array
        Signal to analyse
    ordre: int
        Order of ESPRIT pole estimation

    Returns
    -------
    f: numpy array
        Reduced frequency detected
    zeta: numpy array
        Corresponding damping

    Example
    -------

    >>> x = np.random.randn(1000)
    >>> f,zeta = ESPRIT(x, 20)

    """
    with torch.no_grad():
        s = s.to(device)
        N = len(s)
        H = torch.from_numpy(
            hankel(np.arange(N // 3), np.arange(N // 3, N))).to(device)
        H = s[H.long()]
        H = torch.mm(H, H.transpose(0, 1))
        U, S, V = torch.svd(H, some=False)
        phi = torch.pinverse(U[1:, :ordre]).mm(U[:-1, :ordre])
        z = torch.eig(phi)[0].cpu().numpy()

    z = z[:, 0] + 1j * z[:, 1]
    f = np.angle(z) / (2 * np.pi)
    zeta = -np.log(abs(z))
    torch.cuda.empty_cache()
    return f, zeta

if __name__ == "__main__":
    print("Extracting data...\033[3m")
    os.system("tar -xvf tom.tar.gz")
    print("\033[0mDone! Analysis...\n")
    #device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
    device = torch.device("cpu")

    list_files = glob("*.wav")

    N = 2000  # Fenetre d'analyse
    delta = int(22050 * 50e-3)

    analyse = {}

    for sf in list_files:
        x, fs = li.load(sf, sr=22050)
        peaks = li.util.peak_pick(x, 10000, 10000, 10000, 10000, .05, fs)
        analyse[sf] = []

        for i, peak in enumerate(peaks):
            print("%s, peak %d            " % (sf, i), end="\r", flush=True)
            s = torch.from_numpy(x[peak + delta:peak + delta + N]).float()
            [f, zeta] = ESPRIT(s, 20)

            analyse[sf].append([i, s.numpy(), f * fs, zeta * fs])

    np.save("analyse", analyse)
