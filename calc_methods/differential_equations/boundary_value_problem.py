import numpy as np


def solve_boundary_value_problem(p, q, f, a, b, sigma1, gamma1, delta1, sigma2, gamma2, delta2, sol, n):
    x_range = np.linspace(a, b, n + 1)
    h = (b - a) / n

    f = f(x_range)
    f = -f
    f[0] = delta1
    f[n] = delta2
    a, b, c = np.zeros(n + 1), np.zeros(n + 1), np.zeros(n + 1)
    b[0] = sigma1 - gamma1 / h
    c[0] = gamma1 / h
    a[n] = -gamma2 / h
    b[n] = sigma2 + gamma2 / h
    for i in range(1, n):
        x = x_range[i]
        a[i] = 1 / h ** 2 - p(x) / (2 * h)
        b[i] = -2 / h ** 2 + q(x)
        c[i] = 1 / h ** 2 + p(x) / (2 * h)

    alpha, beta = np.zeros(n + 1), np.zeros(n + 1)
    alpha[0] = -c[0] / b[0]
    beta[0] = f[0] / b[0]
    for i in range(1, n + 1):
        alpha[i] = -c[i] / (a[i] * alpha[i - 1] + b[i])
        beta[i] = (f[i] - a[i] * beta[i - 1]) / (a[i] * alpha[i - 1] + b[i])

    y = np.zeros(n + 1)
    y[n] = (f[n] - a[n] * beta[n - 1]) / (a[n] * alpha[n - 1] + b[n])
    for i in range(n - 1, -1, -1):
        y[i] = alpha[i] * y[i + 1] + beta[i]
    return [x_range, y]
