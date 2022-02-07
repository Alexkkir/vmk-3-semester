import numpy as np
from my_linalg import solution_error


def solve_zeidel(a, b, n_iters=10, omega=1):
    n, m = a.shape
    c = np.zeros((n, n))
    d = np.zeros(n)

    for i in range(n):
        for j in range(n):
            c[i][j] = - a[i][j] / a[i][i] if j != i else 0
    for i in range(n):
        d[i] = b[i] / a[i][i]

    x_prev = np.zeros(n).reshape(-1, 1)
    x = np.zeros(n).reshape(-1, 1)
    min_err = -1
    for iter in range(n_iters):
        for i in range(n):
            x[i] = (1 - omega) * x_prev[i] + \
                   omega * (sum([c[i][j] * x[j] for j in range(i - 1)]) + \
                            sum([c[i][j] * x_prev[j] for j in range(i - 1, n)]) + \
                            d[i])
        x_prev = np.copy(x)
        err = solution_error(a, b, x)
        if min_err == -1 or err < min_err:
            min_err = err
            best_x = x
    return best_x


def solve_relaxation(a, b, **kwargs):
    min_err = -1
    step = 0.01
    n, m = a.shape
    x = np.zeros((n, 1))
    for omega in np.arange(step, 2, step):
        x = solve_zeidel(a, b, omega=omega, **kwargs)
        err = solution_error(a, b, x)
        if min_err == -1 or err < min_err:
            min_err = err
            best_x = x
    return best_x
