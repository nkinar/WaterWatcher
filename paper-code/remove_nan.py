import numpy as np

def remove_nan(x):
    """
    Remove NaN values from array
    """
    y = x[~np.isnan(x)]
    return y

def remove_nan_xy(x, y):
    """
    Function to remove NaN from two arrays
    :param x:           as the x array
    :param y:           as the y array
    :return:            xout, yout
    """
    xt = np.isnan(x)
    yt = np.isnan(y)
    indx = ~np.logical_or(xt, yt)
    xout = x[indx]
    yout = y[indx]
    return xout, yout


def remove_nan_xyz(x, y, z):
    """
    Function to remove NaN from three arrays
    :param x:
    :param y:
    :param z:
    :return:
    """
    xt = np.isnan(x)
    yt = np.isnan(y)
    zt = np.isnan(z)
    indx = ~np.logical_or(np.logical_or(xt, yt), zt)
    xout = x[indx]
    yout = y[indx]
    zout = z[indx]
    return xout, yout, zout


def main():
    x = np.asarray([1, 3, 5, 6, np.NaN, 5, 9, 10])
    y = np.asarray([6, np.NaN, 6, 3, 8, 3, 7, 0])
    z = np.asarray([1, 9, 3, 4, 7, 3, np.NaN, 11])
    xout, yout, zout = remove_nan_xyz(x, y, z)
    print(xout)
    print(yout)
    print(zout)

if __name__ == '__main__':
    main()


