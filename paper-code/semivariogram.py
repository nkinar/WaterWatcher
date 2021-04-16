"""""
Semivariogram.py
Nicholas J. Kinar (n.kinar@usask.ca)

REFERENCES: http://connor-johnson.com/2014/03/20/simple-kriging-in-python/
https://stackoverflow.com/questions/4983258/python-how-to-check-list-monotonicity/4983359

Goovaerts, P. (1997). Geostatistics for Natural Resources Evaluation. New York: Oxford University Press.

Mert, B. A., & Dag, A. (2017). A Computer Program for Practical Semivariogram Modeling and Ordinary Kriging:
A Case Study of Porosity Distribution in an Oil Field. Open Geosciences, 9(1). https://doi.org/10.1515/geo-2017-0050

"""""
# imports
import numpy as np
from get_size import length
from collections.abc import Iterable
from scipy.optimize import minimize
from remove_nan import remove_nan_xyz

# models
SPHERICAL_MODEL = 'spherical'
EXPONENTIAL_MODEL = 'exponential'
GAUSSIAN_MODEL = 'gaussian'

# list of models
MODEL_LIST = [SPHERICAL_MODEL, EXPONENTIAL_MODEL, GAUSSIAN_MODEL]

# default number of elements in the h vector
H_ELEM = 100

############################################################################################

def _edist(x1, y1, x2, y2):
    return np.sqrt((x2-x1)**2 + (y2-y1)**2)


def _monotonic(x):
    dx = np.diff(x)
    return np.all(dx <= 0) or np.all(dx >= 0)


def semivariogram_xz(x, z, h=None, hn=H_ELEM):
    """
    1D semivariogram (for distance or time).
    This is equivalent to having the values distributed along the x axis
    :param x:           x position
    :param z:           magnitude or value
    :param h:           lag vector (None to autoselect)
    :param hn:          number of elements in the lag vector
    :return:
    """
    n = length(x)
    y = np.zeros(n)
    return semivariogram_xyz(x, y, z, h, hn)


def semivariogram_xyz(xin, yin, zin, h=None, hn=H_ELEM):
    """
    Compute the semivariogram at a given lag h
    :param h:           lag distance
    :param xin:           x position
    :param yin:           y position
    :param zin:           magnitude or value
    :param h:           lag vector (None to autoselect)
    :param hn:          number of elements in the lag vector
    :return:            gamma as the vector of semivariances
    The semivariance is always given in units^2
    """
    if length(xin) != length(yin) != length(zin):
        raise Exception('The length of x, y, z must be the same')
    x, y, z = remove_nan_xyz(xin, yin, zin)  # remove any NaN values
    n = length(x)  # obtain the length of x (all others have the same length)
    if h is not None:
        hn = length(h)
        if not isinstance(h, Iterable):
            raise Exception('h must be an iterable array with lag distances')
        if h[0] != 0:
            raise Exception('h[0] must be zero to start at zero lag')
        if not _monotonic(h):
            raise Exception('h must be monotonically increasing')
    if hn <= 2:
        raise Exception('hn <= 2 and hn must be > 2')
    dn = int((n*(n-1))/2)
    d = np.zeros(dn)  # distances
    jvec = np.zeros(dn, dtype=int)
    kvec = np.zeros(dn, dtype=int)
    gamma = np.zeros(hn)                        # semivariance
    gamma_sum_times = np.zeros(hn, dtype=int)   # requried to normalize
    gamma_out = np.zeros(hn)                    # normalized output of gamma
    cnt = 0
    # initial pass to compute the distances and also obtain the max distance if required
    for j in range(n-1):
        for k in range(j+1, n):
            x1 = x[j]
            y1 = y[j]
            x2 = x[k]
            y2 = y[k]
            dist = _edist(x1, y1, x2, y2)
            d[cnt] = dist
            jvec[cnt] = j
            kvec[cnt] = k
            cnt += 1
    if h is None:  # obtain h
        hmax = np.max(d)
        h = np.linspace(0, hmax, hn)
    else:  # h is an input
        hmax = np.max(h)
        hn = length(h)
    hstep = h[1] - h[0]
    for elem in range(dn):  # compute the semivariogram using the vector of distances
        j = jvec[elem]
        k = kvec[elem]
        zj = z[j]
        zk = z[k]
        indxh = int(np.floor(d[elem]/hstep))  # index in lag vector corresponding to distance
        if indxh >= hn:
            continue
        gamma[indxh] += (zj - zk)**2
        gamma_sum_times[indxh] += 1
    for k in range(hn):  # normalize
        if gamma_sum_times[k] == 0:
            continue
        gamma_out[k] = gamma[k] / (2.0*float(gamma_sum_times[k]))
    return h, gamma_out


def _spherical_model(h, a, C0, C1):
    if h > a:
        return C0 + C1
    if a == 0:
        return 0
    ha = h / a
    out = C0 + C1*(1.5 * ha - 0.5 * ha ** 3)
    return out


def spherical_model(h, a, C0, C1):
    """
    :param h:           as the vector of lags
    :param a:           as the range
    :return:
    """
    if not isinstance(h, Iterable):
        return _spherical_model(h, a, C0, C1)
    else:
        n = length(h)
        out = np.zeros(n)
        for k in range(n):
            out[k] = _spherical_model(h[k], a, C0, C1)
        return out

def exponential_model(h, a, C0, C1):
    """
    :param h:               as the vector of lags
    :param a:               as the range
    :return:
    """
    if a == 0:
        return 0
    if a < 1.0e-6:  # prevent overflow
        return 0
    out = C0 + C1*(1 - np.exp(-h/a))
    return out


def gaussian_model(h, a, C0, C1):
    """
    :param h:             as the vector of lags
    :param a:             as the range
    :return:
    """
    if a == 0:
        return 0
    out = C0 + C1*(1 - np.exp((-3*h**2)/(a**2)))
    return out


def fit_model_semivariogram(h, gamma, model):
    """
    Fit model to semivariogram
    :param h:               array of lags
    :param gamma:           semivariance
    :param model:           model to fit to the data
    :return:                a_range, sill, C0, C1, ssd, gamma_model
                            a_range     = the distance on the h axis at which the sill is reached
                            sill        = the distance on the y axis for g(h) that is corresponding to the sill
                            ssd         = sum of squared differences
                            gamma_model = model output
    """
    fm = None
    bounds = None
    if model == SPHERICAL_MODEL:
        fm = spherical_model
    elif model == EXPONENTIAL_MODEL:
        fm = exponential_model
    elif model == GAUSSIAN_MODEL:
        fm = gaussian_model
    else:
        raise Exception('model name not valid')
    def f(vec, *args):
        a = vec[0]
        C0 = vec[1]
        C1 = vec[2]
        mgamma = fm(h, a, C0, C1)
        out = np.sum((gamma-mgamma)**2)  # sum of squared differences (ssd)
        return out
    # Run the minmization to obtain the coefficient
    res = minimize(f, x0=(1, 1, 1))
    out = res.x
    a_range = out[0]
    C0 = out[1]
    C1 = out[2]
    ssd = f([a_range, C0, C1])
    # return the (range, sill, C0, C1, ssd, gamma_model)
    return a_range, fm(a_range, a_range, C0, C1), C0, C1, ssd, fm(h, a_range, C0, C1)


def find_best_model_fit(h, gamma):
    """
    Find the best model to fit to the semivariogram
    :param h:           as the lag
    :param gamma:       as the semivariance
    :return:            (range, sill, ssd, gamma_model, gamma_model_name)
    """
    n = len(MODEL_LIST)
    a_range = np.zeros(n)
    sill = np.zeros(n)
    ssd = np.zeros(n)
    C0 = np.zeros(n)
    C1 = np.zeros(n)
    model = []
    for k in range(n):
        a_range[k], sill[k], C0[k], C1[k], ssd[k], model_y = fit_model_semivariogram(h, gamma, MODEL_LIST[k])
        model.append(model_y)
    indx = np.argmin(ssd)
    return a_range[indx], sill[indx], C0[indx], C1[indx], ssd[indx], model[indx], MODEL_LIST[indx]


############################################################################################

def test_semivar():
    from numpy import genfromtxt
    import matplotlib.pyplot as plt

    fn = './test-data/jura/jura.csv'
    jura_data = genfromtxt(fn, delimiter=',', dtype=float, skip_header=1)
    xcol = 0
    ycol = 1
    zCdcol = 4

    x = jura_data[:, xcol]
    y = jura_data[:, ycol]
    z = jura_data[:, zCdcol]

    n = 20
    h = np.linspace(0, 1.9, n)
    # h = None
    h, gamma = semivariogram_xyz(x, y, z, h=h, hn=H_ELEM)
    # the plot is similar to pg. 101 of Goovaerts (1997)

    model_name = None
    # a_range, sill, C0, C1, ssd, model = fit_model_semivariogram(h, gamma, SPHERICAL_MODEL)
    # a_range, sill, C0, C1, ssd, model  = fit_model_semivariogram(h, gamma, EXPONENTIAL_MODEL)
    # a_range, sill, C0, C1, ssd, model = fit_model_semivariogram(h, gamma, GAUSSIAN_MODEL)
    a_range, sill, C0, C1, ssd, model, model_name = find_best_model_fit(h, gamma)
    print('range = {:.4}'.format(a_range))
    print('sill = {:.4}'.format(sill))
    print('C0 = {:.4}'.format(C0))
    print('C1 = {:.4}'.format(C1))
    print('ssd = {:.4}'.format(ssd))
    if model_name:
        print('model_name = ', model_name)

    plt.figure()
    plt.plot(h, gamma, 'o--', label='data')
    plt.plot(h, model, label='model')
    plt.plot()
    plt.xlabel('h (km)')
    plt.ylabel('Semivariance (ppm^2)')
    plt.ylim((0, 1.2))
    plt.show(block=True)
    # the plot is similar to pg. 101 of Goovaerts (1997)


def test_transect():
    from numpy import genfromtxt
    import matplotlib.pyplot as plt
    fn = './test-data/jura/transects.csv'
    jura_data_transect = genfromtxt(fn, delimiter=',', dtype=float, skip_header=1)
    colx = 0
    colNi = 4
    x = jura_data_transect[:, colx]
    z = jura_data_transect[:, colNi]
    n = 12
    h = np.linspace(0, 1.6, n)
    h, gamma = semivariogram_xz(x, z, h=h, hn=H_ELEM)
    # a_range, sill, C0, C1, ssd, gamma_model = fit_model_semivariogram(h, gamma, 'gaussian')
    model_name = None
    a_range, sill, C0, C1, ssd, gamma_model, model_name = find_best_model_fit(h, gamma)
    print('range = {:.4}'.format(a_range))
    print('sill = {:.4}'.format(sill))
    print('C0 = {:.4}'.format(C0))
    print('C1 = {:.4}'.format(C1))
    print('ssd = {:.4}'.format(ssd))
    print('model name = ', model_name)

    plt.figure()
    plt.plot(h, gamma, 'o--', label='data')
    plt.plot(h, gamma_model, label='model')
    plt.plot()
    plt.xlabel('h (km)')
    plt.ylabel('Semivariance (ppm^2)')
    plt.show(block=True)


############################################################################################

def main():
    # test_semivar()
    test_transect()


if __name__ == '__main__':
    main()

############################################################################################
