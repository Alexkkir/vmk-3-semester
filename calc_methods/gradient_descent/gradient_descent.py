import numpy as np


def error(a, b, x):
    r = a @ x - b
    err = float(r.T @ r)
    grad = 2 * a.T @ r
    return err, grad


def make_step_ray(a, b, x, n_iter=-1):
    f, grad = error(a, b, x)
    eps = 1 / n_iter
    # x_new = x - err['grad']/np.linalg.norm(err['grad']) * alpha
    const = np.linalg.norm(grad) ** 2
    x_new = x - grad / const * f
    return x_new


def make_step_inertia(a, b, x, v, n_iter=-1, g=100000, k=100, t=0.01):
    """
    -v = (fx, fy, fx^2 + fy^2)
    g = (0, 0, -|g|)
    (v, g) = (fx^2 + fy^2) * |g|
    |a| = |g| * cos(A)
    cos(A) = (v, g) / |v| / |g|
    |a| = (v, g) / |v|
    a = v / |v| * |a| = v * (v, g) / |v|^2 = 
        = v * (fx^2 + fy^2) * |g| / |v|^2 = 
        = v * |grad|^2 / |v|^2 * |g|
    grad = (fx, fy)

    |v|^2 = |grad|^2 + |grad|^4
    ---
    2g(f-f') + v^2 = v'^2

    """
    f, grad = error(a, b, x)
    grad_len_2 = np.linalg.norm(grad) ** 2
    e_len_2 = grad_len_2 + grad_len_2 * grad_len_2
    # g = 1000 + f * f
    # k = 10000 * np.exp(-f)

    acc = -grad * grad_len_2 / e_len_2 * g - k * v
    x_new = x + v * t + acc * t ** 2 / 2

    v_new = v + acc * t
    f_new, grad_new = error(a, b, x_new)
    v_new_len_analytical = (v_len_2 + 2 * g * (f - f_new)) ** 0.5

    grad_new_len_2 = np.linalg.norm(grad_new)
    v_new_len_calculated = grad_len_2 + grad_len_2 * grad_len_2
    v_new *= v_new_len / np.linalg.norm(v_new)
    return x_new, v_new
