import numpy as np
from my_linalg import solution_error


def sol_diff(A, b, x, x_delta):
    return solution_error(A, b, x + x_delta) - solution_error(A, b, x)


def gradient_descent(A, b, n_iters=10):
    n, m = A.shape
    x = np.zeros((n, 1))
    epsilon = 0.001
    for iter in range(n_iters):
        x_total_delta = np.zeros((n, 1))
        for i in range(n):
            x_delta = np.zeros((n, 1))
            x_delta[i] = epsilon
            diff_plus = sol_diff(A, b, x, x_delta)
            diff_minus = sol_diff(A, b, x, -x_delta)
            if diff_plus < 0 and diff_minus < 0:
                print(f"diff_plus, diff_minus = {diff_plus}, {diff_minus}")
            if diff_plus < 0:
                x_total_delta += x_delta
            elif diff_minus < 0:
                x_total_delta -= x_delta
        x += x_total_delta
    return x
