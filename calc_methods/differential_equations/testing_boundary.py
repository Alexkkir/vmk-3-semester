import numpy as np
from utils import *


def single_test(method, test, n, draw=False):
    appr = method(test.p, test.q, test.f, test.a, test.b, test.sigma1, test.gamma1, test.delta1, test.sigma2, test.gamma2,
                  test.delta2, test.sol, n)
    if draw:
        r = np.array(appr)[0, :]

        plt.figure(figsize=(7, 7))
        plt.title("aaaa")
        plt.subplot(1, 2, 1)
        print_2d_fun(test.sol, r, comment="Analytical", draw=False)
        plt.subplot(1, 2, 2)
        print_2d_coords(*appr, comment="Solution for problem")
    err = appr_error(test.sol, appr)
    return err
