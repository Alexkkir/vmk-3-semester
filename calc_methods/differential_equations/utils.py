import matplotlib.pyplot as plt
import numpy as np
import itertools


def pairs_to_arr(pairs):
    x = [elem[0] for elem in pairs]
    y = [elem[1] for elem in pairs]
    return [x, y]


def print_2d_coords(x, y=None, comment="Relationship", draw=True):
    if y is None:
        tmp = x
        xx = [elem[0] for elem in tmp]
        yy = [elem[1] for elem in tmp]
        plt.plot(xx, yy)
    else:
        plt.plot(x, y)
    plt.title(f"{comment}")
    plt.xlabel("x")
    plt.ylabel("y")
    if draw:
        plt.show()


def print_2d_fun(f, x_range, comment="f(x, y)", draw=True):
    y = []
    for x in x_range:
        y.append(f(x))
    print_2d_coords(x_range, y, comment, draw=draw)


def print_3d_coords(x, y, z, th=None, animate=False, comment="f(x, y)"):
    if th is not None:
        x_new = []
        y_new = []
        z_new = []
        for x_coord, y_coord, z_coord in x, y, x:
            if abs(z_coord) < th:
                x_new.append(x_coord)
                y_new.append(y_coord)
                z_new.append(z_coord)
        x = x_new
        y = y_new
        z = z_new

    ax = plt.axes(projection='3d')
    ax.plot_trisurf(x, y, z,
                    cmap='viridis', edgecolor='none')

    ax.set_xlabel('x')
    ax.set_ylabel('u')
    ax.set_zlabel(f'{comment}')
    if th is not None:
        ax.set_zlim(-th, th)
    ax.view_init(30, -100)

    if not animate:
        plt.show()
    else:
        for angle in range(0, 360):
            ax.view_init(30, angle)
            plt.draw()
            plt.pause(.001)


def print_3d_fun(f, x_range, y_range, th=None, animate=False, comment="f(x, y)"):
    coords = [elem for elem in itertools.product(x_range, y_range)]

    x = []
    y = []
    z = []
    for pair in coords:
        x.append(pair[0])
        y.append(pair[1])
        z.append(f(*pair))

    if th is not None:
        x_new = []
        y_new = []
        z_new = []
        for x_coord, y_coord, z_coord in x, y, x:
            if abs(z_coord) < th:
                x_new.append(x_coord)
                y_new.append(y_coord)
                z_new.append(z_coord)
        x = x_new
        y = y_new
        z = z_new

    ax = plt.axes(projection='3d')
    ax.plot_trisurf(x, y, z,
                    cmap='viridis', edgecolor='none')

    ax.set_xlabel('x')
    ax.set_ylabel('y')
    ax.set_zlabel(f'{comment}')
    if th is not None:
        ax.set_zlim(-th, th)
    # ax.view_init(30, 10)

    if not animate:
        plt.show()
    else:
        for angle in range(0, 360):
            ax.view_init(30, angle)
            plt.draw()
            plt.pause(.001)


def appr_error(funcs, coords):
    if type(coords) == list and len(coords) == 2:
        f = funcs
        y_true = []
        for x in coords[0]:
            y_true.append(f(x))
        y_true = np.array(y_true)
        y_calc = coords[1]
        mse = ((y_true - y_calc) ** 2).mean()
        return float(mse)
    else:
        coords = np.array(coords).T
        y_calc = coords[1:, :]
        y_true = []
        for f in funcs:
            y_true.append(f(coords[0:1, :]))
        y_true = np.array(y_true).reshape(y_calc.shape)
        mse = ((y_true - y_calc) ** 2).mean()
        return float(mse)
