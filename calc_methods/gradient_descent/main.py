import numpy as np
import gradient_descent
import tests
import graphics
from itertools import product as dot
import matplotlib.pyplot as plt
from scipy.linalg import null_space


def main():
    np.random.seed(2)
    # a, b = tests.random_system(2)
    a = np.diag(np.ones(2))
    b = np.zeros((2, 1))
    print(a)
    print(b)
    sol = np.linalg.solve(a, b)
    print(sol)
    print('================')

    discrete_k = 10
    n = 20 * discrete_k
    accuracy = []
    history = []

    x_start = np.zeros_like(b)
    x_start[0][0] = 0
    x_start[1][0] = 2
    v = np.zeros_like(b)
    v[0][0] = 100
    x = np.copy(x_start)

    accuracy.append(gradient_descent.error(a, b, x)[0])
    history.append(x)
    for i in range(1, n + 1):
        x, v = gradient_descent.make_step_inertia(a, b, x, v, i, k=0, t=0.01 / discrete_k)
        history.append(x)
        accuracy.append(gradient_descent.error(a, b, x)[0])

    print(accuracy[-1])
    graphics.print_2d(range(n + 1), accuracy, lim=None, show=True)

    border_size = 1/3
    history = np.array(history).reshape(-1, 2)
    accuracy = np.array(accuracy)
    diameter = ((sol[0][0] - x_start[0][0]) ** 2 + (sol[1][0] - x_start[1][0]) ** 2) ** 0.5
    center = ((sol[0][0] + x_start[0][0]) / 2, (sol[1][0] + x_start[1][0]) / 2)
    width_r = diameter * border_size
    r1 = np.linspace(min(sol[0][0], x_start[0][0]) - width_r, max(sol[0][0], x_start[0][0]) + width_r, 10)
    r2 = np.linspace(min(sol[1][0], x_start[1][0]) - width_r, max(sol[1][0], x_start[1][0]) + width_r, 10)
    # r1 = np.linspace(sol[0][0] - width_r, sol[0][0] + width_r, 10)
    # r2 = np.linspace(sol[1][0] - width_r, sol[1][0] + width_r, 10)
    # r1 = np.linspace(sol[0][0] / 2 - width_r, sol[0][0] / 2 + width_r, 10)
    # r2 = np.linspace(sol[1][0] / 2 - width_r, sol[1][0] / 2 + width_r, 10)
    coords = np.array([*dot(r1, r2)])
    errors = [gradient_descent.error(a, b, c.reshape(-1, 1))[1].flatten() for c in coords]
    x = [q[0] for q in errors]
    y = [q[1] for q in errors]
    x = -np.array(x)
    y = -np.array(y)
    answer = np.array([[sol[0][0], sol[1][0], 0]])
    errors = np.array([gradient_descent.error(a, b, c.reshape(-1, 1))[0] for c in coords]).flatten()
    average_x_len = (x.T @ x).mean() ** 0.5
    average_y_len = (y.T @ y).mean() ** 0.5
    average_arr_len = (average_x_len ** 2 + average_y_len ** 2).mean() ** 0.5
    k = 2 / average_arr_len * diameter
    graphics.print_3d([
        ('surf', (coords[:, 0], coords[:, 1], errors), {'alpha': 0.5}),
        # ('arr', (coords[:, 0], coords[:, 1], np.zeros_like(coords[:, 0]), x * k, y * k, np.zeros_like(x))),
        ('trace', (history[:, 0], history[:, 1], accuracy)),
        ('dot', (sol[0], sol[1], [0])),
    ], anim=False, angle=(85, 270),
        square=((coords[:, 0].min(), coords[:, 0].max()), (coords[:, 1].min(), coords[:, 1].max())))


if __name__ == '__main__':
    main()
