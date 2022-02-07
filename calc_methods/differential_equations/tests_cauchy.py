import numpy as np


class CauchyProblem:
    def __init__(self, f, init_val, sol):
        self.f = f
        self.init_val = init_val
        self.sol = sol


TestsSingleEquation = [
    CauchyProblem([lambda x, y: np.sin(x) - y],  # 2 variant
                  (0, 10),
                  [lambda x: - 0.5 * np.cos(x) + 0.5 * np.sin(x) + 21 / 2 * np.exp(-x)]),
    CauchyProblem([lambda x, y: -y - x * x],  # задачник
                  (0, 10),
                  [lambda x: -x * x + 2 * x - 2 + 12 * np.exp(-x)])
]
TestsMultipleEquations = [
    CauchyProblem(  # 13 variant
        [lambda x, u, v: np.log(2 * x + (4 * x ** 2 + v ** 2) ** 0.5),
         lambda x, u, v: (4 * x ** 2 + u ** 2) ** 0.5],
        (0, 0.5, 1),
        [lambda x: 0 * x, lambda x: 0 * x]),
    CauchyProblem([lambda t, x, y: y + 1, lambda t, x, y: 2 * np.exp(t) - x],  # example in begining of chapter
                  (0, 2, 0),
                  [lambda t: np.cos(t) + np.exp(t), lambda t: -np.sin(t) + np.exp(t) - 1]),
    CauchyProblem([lambda t, x, y: x + 2 * y, lambda t, x, y: x - 5 * np.sin(t)],  # 834
                  (0, 2, 2),
                  [lambda t: np.exp(-t) + 2 * np.exp(2 * t) - np.cos(t) + 3 * np.sin(t),
                   lambda t: -np.exp(-t) + np.exp(2 * t) + 2 * np.cos(t) - np.sin(t)]),
    CauchyProblem([lambda t, x, y: y - 5 * np.cos(t), lambda t, x, y: 2 * x + y],  # 827
                  (0, -1, 3),
                  [lambda t: -2 * np.sin(t) - np.cos(t), lambda t: np.sin(t) + 3 * np.cos(t)]),
]
