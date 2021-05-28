# http://claudiovz.github.io/scipy-lecture-notes-ES/advanced/interfacing_with_c/interfacing_with_c.html#id12
""" Example of wrapping a C function that takes C double arrays as input using
    the Numpy declarations from Cython """

# import both numpy and the Cython declarations for numpy
import numpy as np
cimport numpy as np

# if you want to use the Numpy-C-API from Cython
# (not strictly necessary for this example)
np.import_array()

# cdefine the signature of our c function
cdef extern from "cython_stuff.h":
    void simple_diagram (float * in_array, int size)

# create the wrapper code, with numpy type annotations
def simple_diagram_func(np.ndarray[float, ndim=1, mode="c"] in_array not None):
    simple_diagram(<float*> np.PyArray_DATA(in_array), in_array.shape[0])
