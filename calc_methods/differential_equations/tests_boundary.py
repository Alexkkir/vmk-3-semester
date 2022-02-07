import math
import numpy as np


class BoundaryValueProblem:
    def __init__(self, p, q, f, a, b, sigma1, gamma1, delta1, sigma2, gamma2, delta2, sol):
        self.p = p
        self.q = q
        self.f = f
        self.a = a
        self.b = b
        self.sigma1 = sigma1
        self.sigma2 = sigma2
        self.gamma1 = gamma1
        self.gamma2 = gamma2
        self.delta1 = delta1
        self.delta2 = delta2
        self.sol = sol


Tests = [
    BoundaryValueProblem(  # var 2
        lambda x: -x,
        lambda x: 2,
        lambda x: x - 1,
        0.5, 0.9,
        1, 2, 1,
        1, -0.5, 2,
        lambda x: 0
    ),
    BoundaryValueProblem(  # 751
        lambda x: 0,
        lambda x: -1,
        lambda x: -2 * x,
        0, 1,
        1, 0, 0,
        1, 0, -1,
        lambda x: math.sinh(x) / math.sinh(1) - 2 * x,
    ),
    BoundaryValueProblem(  # 752
        lambda x: 1 + x * 0,
        lambda x: 0 + x * 0,
        lambda x: -1 + x * 0,
        0, 1,
        0, 1, 0,
        1, 0, 1,
        lambda x: x + np.exp(-x) - np.exp(-1),
    )
]
