import numpy as np
from strcmpi import strcmpi


def is_numpy(a):
    """
    Checks to see if a type is a numpy array or not
    RETURNS
    True = type is a numpy array
    False = type is not a numpy array
    """
    if strcmpi(type(a).__module__, 'numpy'):
        return True
    else:
        return False


def main():
    pass


if __name__ == '__main__':
    main()

