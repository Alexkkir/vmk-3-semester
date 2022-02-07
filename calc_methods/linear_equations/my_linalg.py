from gauss import *


def determinant(A):
    return forward_gauss(np.copy(A))


def inverse(A):
    assert A.shape[0] == A.shape[1]
    n = A.shape[0]
    A_copy = np.copy(A)
    I = np.eye(n)

    M = np.concatenate((A_copy, I), axis=1)
    forward_gauss(M)
    reverse_gauss(M)
    inv_A = M[:, n: 2 * n]
    return inv_A


def norm(A, norm_type='2') -> float:
    if norm_type == '1':
        return np.sum(A, axis=0).max()
    elif norm_type == 'inf':
        return np.sum(A, axis=1).max()
    elif norm_type == '2':
        if len(A.shape) == 1 or len(A.shape) == 2 and (A.shape[0] == 1 or A.shape[1] == 1):
            return np.sum(A ** 2)
        else:
            raise ValueError
    else:
        raise KeyError


def condition_number(A, type):
    return norm(A, type) * norm(inverse(A), type)


def residual(A, b, x):
    return A @ x - b


def my_lstsq(A, b):
    return np.linalg.lstsq(A, b, rcond=None)[0]


def solution_error(A, b, x):
    n, m = A.shape
    return norm(residual(A, b, x), '2') / n ** 0.5


def check_method(method, A, b):
    x = method(A, b)
    return solution_error(A, b, x)
