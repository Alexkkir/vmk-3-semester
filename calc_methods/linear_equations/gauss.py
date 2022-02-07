import numpy as np


def forward_gauss(A):
    """
    Produces triangle matrix
    :param A:  matrix of any shape
    :return: determinant
    """
    n, m = A.shape
    permutations = 0
    for i in range(n):
        if A[i][i] == 0:
            i_swap = i + 1
            while i_swap < n and A[i_swap][i] == 0:
                i_swap += 1
            permutations += 1
            i_swap = min(i_swap, n - 1)
            A[[i, i_swap], :] = A[[i_swap, i], :]

        if A[i][i] != 0:
            A[i, :] /= A[i][i]
        for i_null in range(i + 1, n):
            k = A[i_null][i]
            A[i_null, :] -= A[i, :] * k

    det = 1
    for i in range(n):
        det *= A[i][i]
    if permutations % 2 == 1:
        det *= -1

    return det


def reverse_gauss(A):
    n, m = A.shape
    for i in range(n - 1, -1, -1):
        for i_null in range(i - 1, -1, -1):

            k = 0 if A[i][i] == 0 else A[i_null][i] / A[i][i]
            A[i_null, :] -= A[i, :] * k
    for i in range(n):
        if A[i][i] == 0 and A[i][-1] != 0:
            # print("Solution doesn't exist")
            return np.full(n, np.nan)
    return A[:, -1]


def solve_gauss(A, b):
    A_copy, b_copy = np.copy(A), np.copy(b)
    b_copy = b_copy.reshape(-1, 1)
    M = np.concatenate((A_copy, b_copy), axis=1)
    forward_gauss(M)
    x = reverse_gauss(M)
    x = x.reshape(-1, 1)
    return x


def forward_pivot_gauss(A):
    n, m = A.shape
    unused = [True for _ in range(n)]
    order = []
    for j in range(n):
        max_elem = -1
        i_max = -1
        for i in range(n):
            if unused[i] == True:
                if abs(A[i][j]) > max_elem:
                    max_elem = abs(A[i][j])
                    i_max = i
        i = i_max
        unused[i] = False
        order.append(i)
        if A[i][j] != 0:
            A[i, :] /= A[i][j]
        for i_null in range(n):
            if unused[i_null] == False:
                continue
            k = 0 if A[i][j] == 0 else A[i_null][j] / A[i][j]
            A[i_null, :] -= A[i, :] * k
    return order


def reverse_pivot_gauss(A, order):
    n, m = A.shape
    for j in range(n - 1, -1, -1):
        i = order[j]
        for i_null in range(n):
            if i_null == i:
                continue
            k = 0 if A[i][j] == 0 else A[i_null][j] / A[i][j]
            A[i_null, :] -= A[i, :] * k
    for j in range(n - 1, -1, -1):
        i = order[j]
        if A[i][j] == 0 and A[i][-1] != 0:
            # print("Solution doesn't exist")
            return np.full(n, np.nan)
    return A[order][:, -1]


def solve_pivot_gauss(A, b):
    A_copy, b_copy = np.copy(A), np.copy(b)
    b_copy = b_copy.reshape(-1, 1)
    M = np.concatenate((A_copy, b_copy), axis=1)
    order = forward_pivot_gauss(M)
    x = reverse_pivot_gauss(M, order)
    x = x.reshape(-1, 1)
    return x
