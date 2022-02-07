import matplotlib.pyplot as plt
from gauss import *
from my_linalg import *
import itertools


def random_system(n, type='normal', mu=3, sigma=0.4):
    if type == 'uniform':
        A = np.random.rand(n, n)
        b = np.random.rand(n, 1)
    elif type == 'normal':
        A = np.random.normal(mu, sigma, (n, n))
        b = np.random.normal(mu, sigma, (n, 1))
    return A, b


def random_test(method, n=3, **kwargs):
    A, b = random_system(n)
    x = method(A, b, kwargs)

    delta = residual(A, b, x)
    error = norm(delta, '2') / n ** 0.5
    return error


def test_method(method=solve_gauss, n_arr=(5, 10, 100), n_tests=3):
    errors = []
    for n in n_arr:
        total_err = 0
        for i in range(n_tests):
            err = random_test(method, n)
            total_err += err
        unit_err = total_err / n_tests
        errors.append(unit_err)
    return errors


def test_and_vizualize(method, method_name='', n_arr=range(50, 400, 70), n_tests=10):
    errors = test_method(my_lstsq, n_arr, n_tests)
    plt.plot(n_arr, errors)
    plt.title(f"Computational stability of {method_name}")
    plt.xlabel('Matrix size')
    plt.ylabel('Unit euclid norm of residual')
    plt.show()


def vizualize_residual_error(animate=True):
    np.random.seed(2214322222)
    A, b = random_system(2, type='normal', mu=10, sigma=10)

    r = range(1, 10000, 1000)
    sol = np.linalg.solve(A, b)
    vec0, vec1 = sol[0][0], sol[1][0]

    r = np.arange(-8, 8, 0.1)
    coords = [np.array(elem).reshape(-1, 1) for elem in itertools.product(r, r)]
    # coords = [np.array(elem).reshape(-1, 1) for elem in itertools.product([vec0 - 1, vec0, vec0 + 1], r)]
    # coords = [np.array(elem).reshape(-1, 1) for elem in itertools.product(r, [vec1 - 1, vec1, vec1 + 1])]

    x = []
    y = []
    z = []
    for vec in coords:
        err = solution_error(A, b, vec)
        x.append(*vec[0])
        y.append(*vec[1])
        z.append(err)

    my_lim = 1000
    print(x[0], y[0], z[0])
    z_new = [q if abs(q) < my_lim else my_lim for q in z]
    z = z_new

    ax = plt.axes(projection='3d')
    ax.plot_trisurf(x, y, z,
                    cmap='viridis', edgecolor='none')

    ax.set_xlabel('x')
    ax.set_ylabel('y')
    ax.set_zlabel('||Ax-b||_2 / sqrt(n)')
    ax.set_zlim(0, my_lim)
    ax.view_init(30, 10)

    if animate == False:
        plt.show()
    else:
        for angle in range(0, 360):
            ax.view_init(30, angle)
            plt.draw()
            plt.pause(.001)
