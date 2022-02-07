import numpy as np

Var_1 = \
    (
        (
            np.array([
                [2, 2, -1, 1],
                [4, 3, -1, 2],
                [8, 5, -3, 4],
                [3, 3, -2, 4]
            ], dtype='float'),
            np.array(
                [4, 6, 12, 6],
                dtype='float').reshape(-1, 1)
        ),
        (
            np.array([
                [1, 1, 3, -2],
                [2, 2, 4, -1],
                [3, 3, 5, -2],
                [2, 2, 8, -3]
            ], dtype='float'),
            np.array(
                [1, 2, 1, 2],
                dtype='float').reshape(-1, 1)
        ),
        (
            np.array([
                [2, 5, -8, 3],
                [4, 2, -9, 1],
                [2, 3, -5, -6],
                [1, 8, -7, 0]
            ], dtype='float'),
            np.array(
                [8, 9, 7, 12],
                dtype='float').reshape(-1, 1)
        )
    )

Var_11 = [
    (
        np.array([
            [4, -3, 1, 5],
            [1, -2, -2, 3],
            [3, -1, 2, 0],
            [2, 3, 2, -8]
        ], dtype='float'),
        np.array(
            [7, 3, -1, -7],
            dtype='float').reshape(-1, 1)
    ),
    (
        np.array([
            [2, -2, 1, -1],
            [1, 2, -1, 1],
            [4, -10, 5, -5],
            [2, -14, 7, 11]
        ], dtype='float'),
        np.array(
            [1, 1, 1, -1],
            dtype='float').reshape(-1, 1)
    ),
    (
        np.array([
            [2, -1, 3, 4],
            [4, -2, 5, 6],
            [6, -3, 7, 8],
            [8, -4, 9, 10],
        ], dtype='float'),
        np.array(
            [5, 7, 9, 11],
            dtype='float').reshape(-1, 1)
    )
]

Var_param_1_5 = (np.array(
    [[(i + j) / 50 if i != j else 430 + j / 20 + i / 30 for j in range(30)] for i in range(30)]),
    np.array([[20 * i + 30 for j in range(1)] for i in range(30)])
)
