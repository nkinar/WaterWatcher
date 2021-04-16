import numpy as np
from get_size import length
from collections.abc import Iterable
import matplotlib.pyplot as plt

##################################################################################

def dpoly(ymodel, dx):
    """
    Compute the numerical derivative of a polynomial function
    :param ymodel:      as the vector of y
    :param dx:          as the delta_x (assume regular step)
    :return:            first derivative
    """
    return np.gradient(ymodel, dx)


def epoly(x, coeff):
    """
    Evaluate the polynomial
    :param x:               vector of values
    :param coeff:           vector of coefficients
    :return:                polynomial evaluated at x
    """
    n = length(x)
    y = np.zeros(n)
    for i in range(n):
        for k in range(length(coeff)):
            y[i] += coeff[k]*x[i]**k
    return y


def epolyd(x, coeff):
    """
    Evaluate first derivative of the polynomial
    :param x:           vector of values
    :param coeff:       vector of coefficients
    :return:            polynomial evaluated at x
    """
    n = length(x)
    y = np.zeros(n)
    for i in range(n):
        for k in range(1, length(coeff)):
            y[i] += k*coeff[k]*x[i]**(k-1)
    return y


def solve_constrained_first_stage(x, y, order):
    """
    First stage of constrained polynomial algorithm
    :param x:           the independent variable
    :param y:           the dependent variable
    :param order:       polynomial order
    :return:            vector of coefficients [b0, b1, b2,...]
    """
    # m = polynomial order + 1
    # n = number of data elements as the number of rows
    n = length(x)
    m = order + 1
    # the initial X matrix
    Xmat = np.zeros((n, m))
    Xmat[:, 0] = np.ones(n)
    for k in range(1, m):
        Xmat[:, k] = x**k
    Xt = np.transpose(Xmat)
    beta_hat = np.dot(np.dot(np.linalg.inv(np.dot(Xt, Xmat)), Xt), y)
    return beta_hat


def solve_constrained_second_stage(x, order, dy):
    """
    Second stage of constrained polynomial algorithm
    :param x:           independent values on domain [min(x), max(x)]
    :param order:       polynomial order
    :param dy:          g'(x) as the first derivative of the polynomial
    :return:            coefficients [b1, b2,...] excluding b0
    """
    # m = polynomial order + 1
    # n = number of data elements as the number of rows
    n = length(x)
    m = order
    Xmate = np.zeros((n, m))
    Xmate[:, 0] = np.ones(n)
    for k in range(2, m+1):
        Xmate[:, k-1] = k * x**(k-1)
    ymate = dy
    Xt = np.transpose(Xmate)
    beta_hat_second = np.dot(np.dot(np.linalg.inv(np.dot(Xt, Xmate)), Xt), ymate)
    return beta_hat_second


def constrained_polyfit(x, y, order, inc_or_dec, slack=0.1, ndiscret=None):
    """
    Obtain polyfit for g(x) so that the polynonial is monotonically
    increasing or decreasing on the domain [min(x), max(x)]

    :param x:               the independent variable
    :param y:               the dependent variable
    :param order:           polynomial order

    :param inc_or_dec:      inc to have the polynomial monotonically increasing
                            dec to have the polynonial monotonically decreasing
                            orig to return a canonical polynomial fit

    :param ndiscret:        number of x points to discretize the polynomial on the domain [min(x), max(x)]
                            None to choose the same as the x and y vector size

    :param slack:           g'(x) + abs(slack) > 0 for monotonically increasing polynomial
                            g'(x) - abs(slack) < 0 for monotonically decreasing polynomial

    :return:                vector of coefficients [b0, b1, b2,...] with length(m + 1) for a polynomial with
                            terms in increasing order
    """
    INC_STR = 'inc'
    DEC_STR = 'dec'
    ORIG_STR = 'orig'
    if inc_or_dec !=  INC_STR and inc_or_dec != DEC_STR and inc_or_dec != ORIG_STR:
        raise Exception('inc_or_dec: must be inc or dec or orig')
    n = length(x)
    if n != length(y):
        raise Exception('length(x) != length(y)')
    if inc_or_dec != ORIG_STR and slack == 0.0:
        raise Exception('slack == 0 and this is not allowed since the polynomial will not be '
                        'monotonically increasing or decreasing')
    nd = ndiscret
    if nd == None:
        nd = length(x)
    s = np.abs(slack)
    coeff = solve_constrained_first_stage(x, y, order)
    if inc_or_dec == ORIG_STR:  # original polynomial without monotonically increasing or decreasing
        return coeff
    xpin = np.sort(x)
    xp = np.linspace(xpin[0], xpin[-1], n)
    ymodel_orig_deriv = epolyd(xp, coeff)
    dy = ymodel_orig_deriv
    if inc_or_dec == INC_STR:   # g'(x) > 0
        # dy[dy < 0] = s
        min_val = np.min(ymodel_orig_deriv)
        if min_val < 0:
            dy = ymodel_orig_deriv - min_val + s
        else:
            return coeff  # polynomial is already monotonically increasing
    elif inc_or_dec == DEC_STR:  # g'(x) < 0
        # dy[dy > 0] = -s
        max_val = np.max(ymodel_orig_deriv)
        if max_val > 0:
            dy = ymodel_orig_deriv - max_val - s
        else:
            return coeff  # polynomial is already monotonically decreasing
    coeff_updated = solve_constrained_second_stage(xp, order, dy)
    coeff[1] = coeff_updated[0]  # assign B1* to [B0, B1*, B2, B3,...]
    return coeff


def identify_curve_pieces(x, y, return_points=False):
    """
    Function to identify curve pieces where the curve is concave up or concave down
    Each of these curve pieces are used for piecewise regression.
    :param x:        the independent variable (not used at this time, but might be in the future)
    :param y:        the dependent variable

    :return:        return_points == False
                    [[a0, a1], [a1, a2],...] as a vector of indices corresponding to curve pieces

                    return_points == True
                    [a0, a1, a2,...] as a vector of indices that are monotonically increasing
    """
    if length(x) != length(y):
        raise Exception('The length of x and y must be equal')
    dy = np.gradient(y, 1)          # numerical computation of first derivative (unity spacing)
    dyy = np.gradient(dy, 1)        # numerical computation of second derivative (unity spacing)
    n = length(y)
    xsec = []
    for k in range(1,n):
        if (dyy[k-1] < 0 and dyy[k] > 0) or (dyy[k-1] > 0 and dyy[k] < 0):
            xsec.append(k)
    if return_points:
        return xsec
    xout = []
    if len(xsec) == 0:
        return xout
    elif len(xsec) == 1:
        if xsec[0] == 0 or xsec[0] == n-1:
            return [0, n-1]
        return [[0, xsec[0]], [xsec[0], n-1]]
    xout.append([0, xsec[0]])
    for k in range(1, len(xsec)):
        xout.append([xsec[k-1], xsec[k]])
    xout.append([xsec[-1], n-1])
    for e in xout[:]:
        if e[0] == e[1]:
            xout.remove(e)
    return xout


class PiecewisePoly:
    """
    Piecewise polynomial class to hold intervals and polynonial coefficients for each interval
    """
    def __init__(self, orig_interval, xinterval, coeff):
        self.orig_interval = orig_interval
        self.xinterval = xinterval
        self.coeff = coeff
    def set_orig_interval(self, orig_interval):
        self.orig_interval = orig_interval
    def set_xinterval(self, xinterval):
        self.xinterval = xinterval
    def set_coeff(self, coeff):
        self.coeff = coeff
    def get_orig_interval(self):
        return self.orig_interval
    def get_xinterval(self):
        return self.xinterval
    def get_coeff(self):
        return self.coeff


def find_nearest_idx(array, value):
    """
    Function to find the nearest index to the value in the array
    :param array:       as the array
    :param value:       as the value
    :return:
    REFERENCE: https://stackoverflow.com/questions/2566412/find-nearest-value-in-numpy-array
    """
    array = np.asarray(array)
    idx = (np.abs(array - value)).argmin()
    return idx


def monotonic(x):
    """
    Check if sequence is monotonically increasing or decreasing
    :param x:
    :return:
    REFERENCE: https://stackoverflow.com/questions/4983258/python-how-to-check-list-monotonicity/4983359
    """
    dx = np.diff(x)
    return np.all(dx <= 0) or np.all(dx >= 0)


def evaluatePiecewisePoly(x, poly_in):
    """
    Function to evaluate the piecewise polynomial.
    It is assumed that the piecewise polynomial is in order.

    :param x:           as the independent variable
    :param pp:          as a vector of piecewise polynomial
    :return:
    """
    if not isinstance(x, Iterable):
        raise Exception('x must be a vector of values')
    if not monotonic(x):
        raise Exception('x must be monotonically increasing or decreasing')
    if not isinstance(poly_in, Iterable):
        poly = [poly_in]
    else:
        poly = poly_in
    n = length(x)
    y = np.zeros(n)
    npp = len(poly)
    firstp = poly[0].get_xinterval()
    endp = poly[-1].get_xinterval()
    start = 0
    # check the start of the range for extrapolation
    if x[0] < firstp[0] and x[-1] < firstp[0]:
        npp = 1         # use only the first polynomial
    # check the end of the range for extrapolation
    if x[0] > endp[0] and x[-1] > endp[0]:
        start = npp-1  # use only the last polynomial
    # iterate over all of the polynomials
    for k in range(start, npp):
        p = poly_in[k]
        interval = p.get_xinterval()
        coeff = p.get_coeff()
        # allow for extrapolation
        if k == 0:
            xstarti = 0
        else:
            xstarti = find_nearest_idx(x, interval[0])
        if k == npp-1:
            xendi = n
        else:
            xendi = find_nearest_idx(x, interval[1])
        # obtain the segment over which to evaluate the polynomial
        xseg = x[xstarti:xendi+1]
        # evaluate the polynonial for the segment
        output = epoly(xseg, coeff)
        # assign the output for the segment
        y[xstarti:xendi+1] = output
    return y


def _piecewise_polynomial_regression_different(x, y, order, inc_or_dec, slack, intervals, ndiscret):
    """
    Use different inputs for each curve in polynomial regression
    :param x:                   the independent variable
    :param y:                   the dependent variable
    :param order:               polynomial order vector
    :param inc_or_dec:          inc or dec or orig vector
    :param slack:               slack parameter vector
    :param intervals:           interval vector
    :param ndiscret:            scalar or vector of discretization points on an interval
    :return:                     [PiecewisePoly0, PiecewisePoly1,...] as a list of intervals and
                                coefficients of each interval
    """
    out = []
    n = len(order)
    if n != len(inc_or_dec) != len(slack) != len(intervals):
        raise Exception('The input arrays are not all the same length')
    dsc = isinstance(ndiscret, Iterable)
    if dsc:
        if n != length(ndiscret):
            raise Exception('ndiscret must be the same length as the other arrays')
    for k in range(n):
        e = intervals[k]
        start = e[0]
        end = e[1]
        if dsc:
            nd = ndiscret[k] # obtain element from the vector
        else:
            nd = ndiscret    # scalar
        xx = x[start:end+1]
        yy = y[start:end+1]
        xinterval = [xx[0], xx[-1]]
        coeff = constrained_polyfit(xx, yy, order[k], inc_or_dec[k], slack[k], nd)
        out.append(PiecewisePoly(e, xinterval, coeff))
    return out


def _piecewise_polynomial_regression_same(x, y, order, inc_or_dec, slack, intervals, ndiscret):
    """
    Use the same inputs for polynomial regression over the entire curve
    :param x:                   the independent variable
    :param y:                   the dependent variable
    :param order:               polynomial order
    :param inc_or_dec:          inc or dec or orig
    :param slack:               slack parameter
    :param intervals:           intervals as inputs found by the technique
    :return:                    [PiecewisePoly0, PiecewisePoly1,...] as a list of intervals and
                                coefficients of each interval
    """
    out = []
    for e in intervals:
        start = e[0]
        end = e[1]
        xx = x[start:end+1]
        yy = y[start:end+1]
        xinterval = [xx[0], xx[-1]]
        coeff = constrained_polyfit(xx, yy, order, inc_or_dec, slack, ndiscret)
        out.append(PiecewisePoly(e, xinterval, coeff))
    return out


def piecewise_polynomial_regression(x, y, order, inc_or_dec, slack, intervals=None, ndiscret=None):
    """
    Function to perform piecewise linear regression on the curve
    :param x:           the independent variable
    :param y:           the dependent variable
    :param order:       polynomial order (can be provided as a vector for each interval)
    :param inc_or_dec:  inc or dec or orig: if provided as a vector, intervals cannot be None
    :param slack:       g'(x) + abs(slack) > 0 for monotonically increasing polynomial
                        g'(x) - abs(slack) < 0 for monotonically decreasing polynomial
                        slack is a vector if each interval is to have a different slack
    :param intervals:   None for automatically selected
                        [[a0, a1], [a1, a2],...] as a vector of intervals corresponding to curve pieces
    :return:            [PiecewisePoly0, PiecewisePoly1,...] as a list of intervals and coefficients of each interval
    """
    n = length(x)
    if n != length(y):
        raise Exception('x and y have to be the same length')
    opt_apply_same_all = False
    check1 = np.isscalar(order)
    check2 = isinstance(inc_or_dec, str)
    check3 = np.isscalar(slack)
    if check1 and check2 and check3:    # apply the same inputs to all of the curves
        opt_apply_same_all = True
    if not opt_apply_same_all:          # apply different inputs to each of the piecewise curves
        check4 = isinstance(order, Iterable)
        check5 = isinstance(inc_or_dec, Iterable)
        check6 = isinstance(slack, Iterable)
        check7 = isinstance(intervals, Iterable)
        if ndiscret is not None:
            check8 = isinstance(ndiscret, Iterable)
        else:
            check8 = True
        check9 = check4 and check5 and check6 and check7 and check8
        if not check9:
            raise Exception('All inputs must be lists, tuples or numpy arrays')
    if intervals is None:
        intervals = identify_curve_pieces(x, y, return_points=False)
    if not intervals:  # empty list of intervals
        intervals = [[0, n-1]]  # include all of the elements in the polynomial fit
    if opt_apply_same_all:   # apply the same inputs to all of the curves
        return _piecewise_polynomial_regression_same(x, y, order, inc_or_dec, slack, intervals, ndiscret)
    # else apply different inputs to each of the piecewise curves
    return _piecewise_polynomial_regression_different(x, y, order, inc_or_dec, slack, intervals, ndiscret)


#####################################################################################
# Development Tests (Examples)
#####################################################################################

def test2():
    n = 100

    slack = 1
    inc_or_dec = 'inc'
    # inc_or_dec = 'dec'

    x = 2.3 - 3.4 * np.random.normal(0, 1, n)
    y = x - 2.5 * (x ** 2) + 0.8 * (x ** 3) + np.random.normal(-4, 4, n)

    if  inc_or_dec == 'dec':
        y = -y

    order = 5
    xpin = np.sort(x)
    xp = np.linspace(xpin[0], xpin[-1], n)
    coeff = solve_constrained_first_stage(x, y, order)
    coeff_compare = polyfit(x, y, order)
    ymodel_orig = epoly(xp, coeff)
    ymodel_orig_deriv = epolyd(xp, coeff)

    coeff_updated = constrained_polyfit(x, y, order, inc_or_dec, slack)
    print('First set of coefficients:')
    print(coeff)
    print('Updated coeff:')
    print(coeff_updated)

    # compute the updated models
    ymodel_updated = epoly(xp, coeff_updated )
    ymodel_udpated_deriv = epolyd(xp, coeff_updated)

    plt.figure()
    plt.title('Curve')
    plt.scatter(x, y, label='original', color='blue')
    plt.plot(xp, ymodel_orig, label='polyfit', color='green')
    plt.plot(xp, ymodel_updated, label='polyfit updated', color='orange')
    plt.xlabel('x')
    plt.ylabel('y')
    plt.legend()
    block = False
    plt.show(block=block)

    plt.figure()
    plt.title('Derivative')
    plt.plot(xp, ymodel_orig_deriv, label='original')
    plt.plot(xp,  ymodel_udpated_deriv, label='polyfit updated', color='orange')
    plt.legend()
    block = True
    plt.show(block=block)


def test1():
    def gf(xin):
        x = xin[0]
        y = xin[1]
        out = [x**2 - y, 2*x**2+y-12]
        return np.asarray(out)
    def jf(xin):
        x = xin[0]
        y = xin[1]
        out = [[2*x-1, -1], [4*x, 1]]
        return np.asarray(out)
    xs = np.asarray([1.3, 3.2])
    n = 2
    yc = np.ones(n) * 9
    # yc = np.zeros(n)
    iternum = 10
    xout =  solve_constrained_newton_lt(xs, gf, jf, yc, iternum)
    print('xout = ')
    print(xout)
    print('gf = ')
    print(gf(xout))


##################################################################################


def main():
    pass

if __name__ == '__main__':
    main()
