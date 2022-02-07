from tabulate import tabulate
import tests_cauchy
import cauchy_problem
from utils import *


def single_test(method, test, h=0.001, n_it=1000, draw=False, comment="Runge kutta"):
    appr = method(test.f, test.init_val, h, n_it)
    err = appr_error(test.sol, appr)
    if draw:
        r = np.array(appr)[0, :]
        appr = pairs_to_arr(appr)
        # print_2d_fun(test.sol, r, comment="Analytical solution")
        print_2d_coords(*appr, comment=comment)
    return err


def test_cauchy_problem():
    h = 0.001
    n_it = 1000
    print(f"Parameters: h={h}, n_it={n_it}")

    table = []
    headers = ['Degree 2', 'Degree 4']
    params = {"comments": ["System from 1 equation:", "System from 2 equations:"],
              "tests": [tests_cauchy.TestsSingleEquation, tests_cauchy.TestsMultipleEquations]}
    for comment, tests_set in zip(params['comments'], params['tests']):
        print(comment)
        for test in tests_set:
            table.append([single_test(cauchy_problem.solve_runge_kutta_2, test, h, n_it),
                          single_test(cauchy_problem.solve_runge_kutta_4, test, h, n_it)])
        print(tabulate(table, headers, 'latex'))
        print()
        table = []
