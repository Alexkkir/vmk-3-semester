import numpy as np
import matplotlib.pyplot as plt
from tests import *
from gauss import *
from testing import *
from zeidel import *
from my_linalg import *
from gradient_descent import *
from itertools import product as dot
from mpl_toolkits.mplot3d import Axes3D
from tabulate import tabulate
from tables import *

A, b = Var_1[0]
print(condition_number(A, 'inf'))
