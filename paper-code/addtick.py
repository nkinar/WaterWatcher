import matplotlib.pyplot as plt
import numpy as np

def addtick_x(x, clear=False):
    """
    Add a tick to the x axis
    :param x:
    :param clear:
    :return:
    """
    ax = plt.gca()
    xt = ax.get_xticks()
    xt = np.append(xt, x)
    ax.set_xticks(xt)
    if clear:
        s = [''] * len(xt)
        s[-1] = str(x)
        ax.set_xticklabels(s)


def addtick_y(y, clear=False):
    """
    Add a tick to the y axis
    :param x:
    :return:
    """
    ax = plt.gca()
    yt = ax.get_yticks()
    yt = np.append(yt, y)
    ax.set_yticks(yt)
    if clear:
        s = [''] * len(yt)
        s[-1] = str(y)
        ax.set_yticklabels(s)

