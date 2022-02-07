import numpy as np


def random_system(n=2, mu=10, sigma=10):
    a = np.random.normal(mu, sigma, (n, n))
    shape_b = (n, 1)
    b = np.random.normal(mu, sigma, shape_b)
    return a, b
