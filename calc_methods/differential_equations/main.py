import numpy as np
import matplotlib.pyplot as plt

from cauchy_problem import *
from boundary_value_problem import *
import tests_cauchy
import tests_boundary
import testing_cauchy
import testing_boundary
import utils

# test = tests_boundary.Tests[2]
# print(testing_boundary.single_test(solve_boundary_value_problem, test, n=100, draw=True))
test = tests_cauchy.TestsMultipleEquations[0]
# print(testing_cauchy.single_test(solve_runge_kutta_2, test, draw=True, comment='Runge-Kutta 2'),
#       testing_cauchy.single_test(solve_runge_kutta_4, test, draw=True, comment='Runge-Kutta 4'))
# testing_cauchy.test_cauchy_problem()
appr = solve_runge_kutta_2(test.f, test.init_val, 0.001, 1000)
x = [q[0] for q in appr]
y = [q[1] for q in appr]
z = [q[2] for q in appr]

utils.print_3d_coords(x, y, z, comment='v')

