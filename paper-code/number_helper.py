#!/usr/bin/env python3
import numpy as np

# Checks if something is a number

def is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        return False


def get_closest_date(date, dates):
    """
    Get the closest date to dates
    :param date:    as the date
    :param dates:   as the list or numpy array of dates
    :return:        d, indx
                    d as the date
                    indx as the index of the date
    """
    d = min(dates, key=lambda d: abs(d - date))
    indx = 0
    if isinstance(dates, np.ndarray):
        indx = np.where(dates == d)[0]
    elif isinstance(dates, list):
        indx = dates.index(d)
    else:
        raise ValueError('dates is not a list nor a numpy array')
    return d, indx


