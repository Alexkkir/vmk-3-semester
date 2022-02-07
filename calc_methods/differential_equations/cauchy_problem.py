import numpy as np


def solve_runge_kutta_2(funcs, init_val, h, n_iters):
    n = len(init_val)
    coords = [init_val]
    for iter in range(n_iters):
        prev_coords = coords[-1]
        x = prev_coords[0]
        x_new = x + h
        aux_coords = [x_new]
        new_coords = [x_new]
        for i in range(n - 1):
            y = prev_coords[i + 1]
            y_aux = y + h * funcs[i](*prev_coords)
            aux_coords.append(y_aux)
        for i in range(n - 1):
            y = prev_coords[i + 1]
            y_new = y + h / 2 * (funcs[i](*prev_coords) + funcs[i](*aux_coords))
            new_coords.append(y_new)
        coords.append(new_coords)
    return coords


def solve_runge_kutta_4(funcs, init_val, h, n_iters):
    n = len(init_val)
    coords = [init_val]
    for iter in range(n_iters):
        prev_coords = coords[-1]
        x = prev_coords[0]
        x_new = x + h
        new_coords = [x_new]
        aux_coords_1 = [x + h / 2]
        aux_coords_2 = [x + h / 2]
        aux_coords_3 = [x + h]

        for i in range(n - 1):  # aux coords 1
            y = prev_coords[i + 1]
            k1 = funcs[i](*prev_coords)
            y_aux = y + h / 2 * k1
            aux_coords_1.append(y_aux)

        for i in range(n - 1):  # aux 2
            y = prev_coords[i + 1]
            k2 = funcs[i](*aux_coords_1)
            y_aux = y + h / 2 * k2
            aux_coords_2.append(y_aux)

        for i in range(n - 1):  # aux coords 3
            y = prev_coords[i + 1]
            k3 = funcs[i](*aux_coords_2)
            y_aux = y + h * k3
            aux_coords_3.append(y_aux)

        for i in range(n - 1):  # final coords
            y = prev_coords[i + 1]
            k1 = funcs[i](*prev_coords)
            k2 = funcs[i](*aux_coords_1)
            k3 = funcs[i](*aux_coords_2)
            k4 = funcs[i](*aux_coords_3)
            y_new = y + h / 6 * (k1 + 2 * (k2 + k3) + k4)
            new_coords.append(y_new)
        coords.append(new_coords)
    return coords
