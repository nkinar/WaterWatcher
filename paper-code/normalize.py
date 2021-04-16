#!/usr/bin/env python3
import numpy as np
from get_size import length, get_size

# """
# Collection of functions for normalization
# REFERENCES
# [1] H. Lohninger, "Teach/Me Data Analysis", Springer, 1999
# [2] M. Smithson and J. Verkuilen, “A better lemon squeezer? Maximum-likelihood regression with beta-distributed
# dependent variables.,”
# Psychological Methods, vol. 11, no. 1, pp. 54–71, 2006.
# [3] M. Smithson and J. Verkuilen, “Supplemental Material for A Better Lemon Squeezer?
# Maximum-Likelihood Regression With Beta-Distributed Dependent Variables.,” Psychological Methods, 2006.
# """

def normalize_open_interval(x,normalized=False,s=0.5):
    """
    Function to normalize data on the open interval (0,1)
    INPUTS
    x = data vector
    normalized = True if the function has already been normalized
    s = Bayesian estimator = 0.5 (see paper [3] for details)
    REFERENCES: [2],[3]
    """
    if(normalized is False):
        x0 = normalize_regular(x)
    else:
        x0 = x
    N = length(x0)
    y = (x0*(N-1) + s)/N
    return y
    


def normalize_regular(x):
    """
    Function to normalize data on [0,1] interval
    REFERENCES: [1]
    """
    return normalize(x, 0, 1)


def normalize_matrix_cols(M, Rmin, Rmax, Dmin=None, Dmax=None):
    """
    Function to normalize the matrix by the cols
    """
    m, n = get_size(M)
    Mout = np.zeros([m, n])
    for k in range(n):
        Mout[:, k] = normalize(M[:, k], Rmin, Rmax, Dmin, Dmax)
    return Mout

def normalize_matrix(a, amin=None, amax=None):
    b = (a - np.min(a)) / np.ptp(a)
    if amin:
        b += amin
    if amax:
        b *= amax
    return b


def normalize(x, Rmin, Rmax, Dmin=None, Dmax=None):
    """
    PURPOSE
    Function to normalize data on [Rmin,Rmax] interval 
    INPUTS
    x = input signal
    Rmin = minimum number 
    Rmax = maximum number
    OUTPUT
    y = normalized output
    REFERENCES: [1]
    """
    x0 = np.asarray(x)  # ensure that the input is a numpy array
    if Dmin is None:
        Dmin = np.min(np.min(x0))
    if Dmax is None:
        Dmax = np.max(np.max(x0))

    s0 = (Rmax - Rmin) / (Dmax - Dmin)
    s1 = (Rmin*Dmax - Rmax*Dmin) / (Dmax - Dmin)
    y = x0*s0 + s1
    return y

def test_func():
    x = np.asarray([1,-2,3,-4,-5,6,7,8,9,10])
    print(x)
    print(normalize_regular(x))
    print(normalize_open_interval(x))
    
def main():
    test_func()

if __name__ == '__main__': main()
