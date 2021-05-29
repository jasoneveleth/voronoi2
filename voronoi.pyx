# import both numpy and the Cython declarations for numpy
import numpy as np
cimport numpy as np

# if you want to use the Numpy-C-API from Cython
# (not strictly necessary for this example)
np.import_array()

# cdefine the signature of our c function
cdef extern from "cython_stuff.h":
    void simple_diagram(float *edges, int size)

# wrapper function. 
# ::1 in the last axis makes 'arr' contiguous in memory, and the rest of the type
# definition make 'arr' a memory view. To call the c function we need a pointer
# though, which we can get by making a point to the first element of the memory
# view. The 'not None' tells Cython that I never intend to pass None, so it can
# turn off some checks for speed
def simple_diagram_func(float[:,:,::1] arr not None):
    simple_diagram(&arr[0,0,0], arr.shape[0])
