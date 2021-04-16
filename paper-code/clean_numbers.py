#!/usr/bin/env python3
import numpy as np

def clean_numbers(x,setNum = 0):
    """
    Function to clean dataset from NaN or Inf
    """
    x[np.isnan(x)] = setNum
    x[np.isinf(x)] = setNum
    return x

def test():
    a = np.asarray( [1,2,3,4,np.NaN,5,6,7,np.Inf,8,-np.Inf,10] )   
    print(a)
    print(clean_numbers(a))
    
def main():
    test()
    
if __name__ == '__main__': main()