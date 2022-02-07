from tabulate import tabulate
from tests import *
from gauss import *
from zeidel import *
from my_linalg import *

def latex_matrix(m):
    s = ""
    s += r"\["
    s += r"\left["
    s += tabulate(m, tablefmt="latex_raw").replace("tabular", "array").replace("\n\hline", "")
    s += r"\right]"
    s += r"\]"
    return s

def text_to_arr(x):
    if type(x) == np.float64:
        x = "%.5g" % x
    s = ""
    s += r"\["
    s += r"\begin{array}{c}"
    s += rf"\text{{{str(x)}}}"
    s += r"\end{array}"
    s += r"\]"
    return s

def make_table(tests, vals, comment=r"$\|Ax-b\|_2 / \sqrt{n}$"):
    str_table = []
    for (A, b), v in zip(tests, vals):
        str_table.append([latex_matrix(A), latex_matrix(b), text_to_arr(v)])

    header = [["\\textbf{A}", "\\textbf{b}", comment]]

    code = "\\begin{center}"
    code += "\\begin{tabular}{|*3{>{\centering\\arraybackslash}p{.3\\textwidth}|}}\n"
    code += "\\hline\n"
    for line in header + str_table:
        for cell in line[:-1]:
            code += cell
            code += "\n&\n"
        code += line[-1]
        code += "\n\\\\ \\hline\n"

    code += "\\end{tabular}"
    code += "\\end{center}"
    return code

# method = solve_gauss
# tests = Var_1
# vals = [check_method(method=method, A=A, b=b) for A, b in tests]
# code = make_table(tests, vals, r"$\|Ax-b\|_2 / \sqrt{n}$")
# print(code)

