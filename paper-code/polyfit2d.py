# REFERENCE
# https://stackoverflow.com/questions/33964913/equivalent-of-polyfit-for-a-2d-polynomial-in-python
# https://github.com/tkanmae/polynomial2d/blob/master/polynomial2d/polynomial.py
# https://stackoverflow.com/questions/31081232/faster-way-to-attach-2d-polynomial-coefficients-to-terms-in-python
import numpy as np
from collections.abc import Iterable
from get_size import length

"""
fig = plt.figure()
ax = Axes3D(fig)
surf = ax.plot_trisurf(xp, yp, zp, cmap=cm.jet, linewidth=0.1)
fig.colorbar(surf, shrink=0.5, aspect=5)
plt.show()
"""


def polyfit2d_evaluate(x, y, c, indices):
    """
    Evaluate 2D polynomial using scalar inputs
    :param x:           as a scalar x
    :param y:           as a scalar x
    :param c:           as a coefficient matrix
    :param indices:     as the indices associated with each coefficient
    :return:    scalar output
    """
    n = length(c)
    out = 0
    for k in range(n):
        ii = indices[k][0]
        jj = indices[k][1]
        out += c[k]*(x**ii)*(y**jj)
    return out


def polyfit2d_evaluate_vec(x, y, c, indices):
    """
    Evaluate 2D polynomial using vector x, y inputs
    :param x:           as x input vector
    :param y:           as y input vector
    :param c:           as coefficients
    :param indices:     as the polynomial indices (i,j) associated with each coefficient
    :return: vector of evaluated outputs
    """
    if not isinstance(x, Iterable) and not isinstance(y, Iterable):
        x = np.asarray([x,])
        y = np.asarray([y,])
    nx = length(x)
    ny = length(y)
    if nx != ny:
        raise Exception('nx != ny')
    n = nx
    out = np.zeros(n)
    for k in range(n):
        out[k] = polyfit2d_evaluate(x[k], y[k], c, indices)
    return out


def polyfit2dsurf(x, y, kx, ky):
    """
    Use plt.matshow(fitted_surf) to visualize the surface
    :param x:
    :param y:
    :param kx:
    :param ky:
    :return:
    """
    fitted_surf = np.polynomial.polynomial.polyval2d(x, y, soln.reshape((kx + 1, ky + 1)))
    return fitted_surf


def polyfit2d(xin, yin, zin, kx=3, ky=3, order=None):
    '''
    Two dimensional polynomial fitting by least squares.
    Fits the functional form f(x,y) = z.

    Notes
    -----
    Resultant fit can be plotted with:
    np.polynomial.polynomial.polygrid2d(x, y, soln.reshape((kx+1, ky+1)))

    Parameters
    ----------
    x, y: array-like, 1d
        x and y coordinates.
    z: np.ndarray, 2d
        Surface to fit.
    kx, ky: int, default is 3
        Polynomial order in x and y, respectively.
    order: int or None, default is None
        If None, all coefficients up to maxiumum kx, ky, ie. up to and including x^kx*y^ky, are considered.
        If int, coefficients up to a maximum of kx+ky <= order are considered.

    Returns
    -------
    coefficients
    list of tuples with (i, j)
    where x**i
    whwere y**j
    '''
    x = xin
    y = yin
    z = zin
    # grid coords
    x, y = np.meshgrid(x, y)
    # coefficient array, up to x^kx, y^ky
    coeffs = np.ones((kx+1, ky+1))

    # solve array
    a = np.zeros((coeffs.size, x.size))

    # for each coefficient produce array x^i, y^j
    lst_index = []
    for index, (j, i) in enumerate(np.ndindex(coeffs.shape)):
        # do not include powers greater than order
        if order is not None and i + j > order:
            arr = np.zeros_like(x)
        else:
            arr = coeffs[i, j] * x**i * y**j
        a[index] = arr.ravel()
        lst_index.append((i,j))

    # do leastsq fitting and return leastsq result
    out = np.linalg.lstsq(a.T, np.ravel(z), rcond=None)
    return out[0], lst_index


def main():
    pass

if __name__ == '__main__':
    main()