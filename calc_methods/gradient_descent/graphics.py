import matplotlib.pyplot as plt
import numpy as np


def print_3d(plots, comment='z', th=None, anim=False, angle=(30, 65), show=True, square=None) -> None:
    ax = plt.axes(projection='3d')

    for plot in plots:
        g_type = plot[0]
        data = plot[1]
        g_kwargs = {} if len(plot) < 3 else plot[2]

        if g_type == 'surf':
            ax.plot_trisurf(*data,
                            cmap='viridis', edgecolor='none', **g_kwargs)
        elif g_type == 'trace':
            start_point = (data[0][0], data[1][0], data[2][0])
            ax.scatter(*start_point, color='red')
            ax.plot(*data, 'o-', color='black', linewidth=1, **g_kwargs)
        elif g_type == 'line':
            ax.plot(*data, **g_kwargs)
        elif g_type == 'dot':
            ax.scatter(*data, **g_kwargs, color='blue')
        elif g_type == 'arr':
            ax.quiver(*data, **g_kwargs)

    ax.set_xlabel('x')
    ax.set_ylabel('y')
    ax.set_zlabel(comment)
    if square is not None:
        ax.set_xlim(square[0][0], square[0][1])
        ax.set_ylim(square[1][0], square[1][1])
    ax.view_init(*angle)
    if show:
        if not anim:
            plt.show()
        else:
            for angle in range(0, 360):
                ax.view_init(30, angle)
                plt.draw()
                plt.pause(.001)


def print_2d(x, y, comment='y', show=True, lim=None):
    if lim is not None:
        plt.ylim(0, lim)
    plt.plot(x, y, 'o-')
    plt.xlabel('x')
    plt.ylabel(comment)
    if show:
        plt.show()
