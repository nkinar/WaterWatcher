#!/usr/bin/env python3
import matplotlib.pyplot as plt
import numpy as np
from clean_numbers import clean_numbers
import matplotlib as mpl

def imagesc(Umat,x=None,y=None,**kwargs):
    """
    Replacement for imagesc function
    """
    plt.figure()
    if (x is not None and y is not None):
        plt.imshow(Umat, extent =(x[0], x[-1], y[0], y[-1]), aspect = 'auto', interpolation = 'nearest', **kwargs)
    else:
        plt.imshow(Umat, aspect = 'auto', interpolation='nearest', **kwargs)
    return plt
    
       
def abs_imagesc(Umat,x=None,y=None,**kwargs):
    """
    Plot abs value of matrix
    """
    return imagesc(np.abs(Umat),x,y,**kwargs)


def power_imagesc(Umat,x=None,y=None,log_power=False,**kwargs):
    """
    Plot power value of matrix as |U|^2
    """
    a = np.abs(Umat)**2
    if(log_power):
        a = np.log( a ) # warning: if values are close to zero
        a = clean_numbers(a)
    return imagesc(a,x,y,**kwargs)


def turn_ticklabels_off_xaxis():
    cur_axes = plt.gca()
    cur_axes.axes.get_xaxis().set_ticklabels([])

def turn_ticklabels_off_yaxis():
        cur_axes = plt.gca()
        cur_axes.axes.get_yaxis().set_ticklabels([])

def make_axes_tight():
    mpl.rcParams['axes.autolimit_mode'] = 'round_numbers'
    mpl.rcParams['axes.xmargin'] = 0
    mpl.rcParams['axes.ymargin'] = 0

def set_figure_text_size(size):
    mpl.rcParams.update({'font.size': size})

def use_ggplot_style():
    plt.style.use('ggplot')

