import numpy as np
import pickle
from polyfit2d import polyfit2d_evaluate_vec
from collections.abc import Iterable
from is_numpy import is_numpy
from get_size import length
from collections.abc import Iterable
from constants import *


class WaterWatcherProcess:
    def __init__(self):
        self.d = pickle.load(open(FN_CALIB_COEFF, "rb"))
        self.TURBIDITY_MODEL_OBJ = self.d[TURBIDITY_MODEL_OBJ]
        self.TDS_MODEL_COEFF = self.d[TDS_MODEL_COEFF]
        self.TDS_MODEL_INDICES = self.d[TDS_MODEL_INDICES]

    def constrain(self, out, min_val, max_val):
        if isinstance(out, Iterable) and not is_numpy(out):
            out = np.asarray(out)
        if is_numpy(out):
            out[out < min_val] = min_val
            out[out > max_val] = max_val
            if length(out) == 1 and isinstance(out, Iterable):
                return float(out)
            return out
        if out < min_val:
            return min_val
        elif out > max_val:
            return max_val
        return out

    def turbidity(self, v):
        out = self.TURBIDITY_MODEL_OBJ(v)
        return out

    def tds(self, v, temp):
        out = polyfit2d_evaluate_vec(v, temp, self.TDS_MODEL_COEFF, self.TDS_MODEL_INDICES)
        return out
