import numpy as np
from scipy.interpolate import RegularGridInterpolator

class RegularGridInterpolatorNNextrapol:
    """A regular grid interpolator that uses the value of the nearest point when
    called outside of its bounds.
    """
    def __init__(self, points, values, method='linear'):
        self.interp = RegularGridInterpolator(points, values, method=method,
                                           bounds_error=False, fill_value=np.nan)
        self.nearest = RegularGridInterpolator(points, values, method='nearest',
                                           bounds_error=False, fill_value=None)

    def __call__(self, xi):
        vals = self.interp(xi)
        idxs = np.isnan(vals)
        nears = self.nearest(xi)
        vals[idxs] = nears[idxs]
        return vals