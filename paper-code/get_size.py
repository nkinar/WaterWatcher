#!/usr/bin/env python3
import numpy as np
from is_numpy import is_numpy


def length(A):
    """
    Function to get the longest length of a matrix or vector
    """
    dim = get_size(A)
    return int(np.max(dim))


def check_if_numpy(x):
    """
    Check if an object is a numpy object
    :param x:
    :return:
    """
    if 'numpy' in str(type(x)):
        return true
    return false


def is_vector(A):
    """
    Checks to see if at least one dimension is == 1
    """
    (m,n) = get_size(A)
    logic = (m == 1 or n == 1)
    if (logic):
        return True
    else:
        return False


def is_matrix(A):
    """
    Checks to see if this is a 2D matrix 
    """
    return not is_vector(A)
    
      
def get_size(A):
    """
    Function that will always return two values (m,n) for the shape
    of an array
    INPUT
    A = vector or 2D array
    OUTPUTS
    m = rows
    n = cols
    """
    if not is_numpy(A):
        raise ValueError('Input must be a numpy type')
    m = 1 
    n = 1
    siz = A.shape
    if len(siz) == 0:
        m = 1
    elif(len(siz) == 1):      # vector
        m = siz[0]
    elif(len(siz) == 2):    # array
        m = siz[0]
        n = siz[1]
    else:
        raise TypeError('The get_size function will only work for vectors or 2D arrays created in numpy')
    return (m,n)

#############################################################################################################

def test_size():
    """
    Test function for this file
    """
    A = np.zeros([2,2]) 
    (m,n) = get_size(A)
    print('m  = ', m , 'n = ', n)
    a = np.zeros(10)
    (m1,n1) = get_size(a)
    print('m1  = ', m1 , 'n1 = ', n1)
    print('A is vector = ', is_vector(A))
    print('a is vector = ', is_vector(a))
    print('A is matrix = ', is_matrix(A))
    print('length of 3 x 4 array  = ', length(np.zeros([3,4])))

def main():
    test_size()

if __name__ == '__main__': main()
    
    