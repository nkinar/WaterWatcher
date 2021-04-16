import numpy as np
from get_size import length
from sklearn.metrics import mean_squared_error
import datetime
import scipy.interpolate as interp


def compute_rmsd(y_actual, y_predicted):
    return np.sqrt(mean_squared_error(y_actual, y_predicted))


def compute_mb(known, model):
    """
    Computes the Mean Bias (MB) and returns this in the same units as the data
    :param known:
    :param model:
    :return:
    """
    k = np.asarray(known)
    m = np.asarray(model)
    n = k.size
    term = np.average(m-k)
    return term


def _convert_to_timestamp(x):
    out = []
    for k in x:
        out.append(k.timestamp())
    return np.asarray(out)


def compute_rmsd_mb_arrays(x1, y1, x2_known, y2_known):
    """
    Compute the RMSD and MB arrays
    :param x1:                  as the dataset x to be compared (timestep)
    :param y1:                  as the dataset y to be compared (values)
    :param x2_known:            as the known dataset x to be compared (timestep)
    :param y2_known:            as the known dataset y to be compared (values)
    :return:
    """
    n1 = length(x1)
    n2 = length(x2_known)
    if n1 != length(y1):
        raise ValueError('x1 and y1 need to be the same length')
    if n2 != length(y2_known):
        raise ValueError('x2_known and y2_known need to be the same length')
    x1_t = _convert_to_timestamp(x1)
    x2_t = _convert_to_timestamp(x2_known)
    if n1 > n2:
        xsamp = x2_t
    else:
        xsamp = x1_t
    fx1 = interp.interp1d(x1_t, y1, fill_value='extrapolate')
    fx2 = interp.interp1d(x2_t, y2_known, fill_value='extrapolate')
    y1_interp = fx1(xsamp)
    y2_interp = fx2(xsamp)
    mb = compute_mb(y2_interp, y1_interp)
    rmse = compute_rmse(y2_interp, y1_interp)
    return rmse, mb



