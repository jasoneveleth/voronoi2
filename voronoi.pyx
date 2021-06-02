# import both numpy and the Cython declarations for numpy
import numpy as np
cimport numpy as np

# if you want to use the Numpy-C-API from Cython
# (not strictly necessary for this example)
np.import_array()

# cdefine the signature of our c function
cdef extern from "src/cython_stuff.h":
    void simple_diagram(float *edges, int size, float *sites, int nsites)

cdef extern from "src/cython_stuff.h":
    void gradient_descent(float *edges, float *sites, float *perimeter, float jiggle, int sites, int pts_per_trial, int trials)

cdef struct point:
    float x
    float y

# wrapper function. 
# ::1 in the last axis makes 'arr' contiguous in memory, and the rest of the type
# definition make 'arr' a memory view. To call the c function we need a pointer
# though, which we can get by making a point to the first element of the memory
# view. The 'not None' tells Cython that I never intend to pass None, so it can
# turn off some checks for speed
def simple_diagram_func(float[:,:,::1] arr not None, float[:,::1] sites not None):
    simple_diagram(&arr[0,0,0], arr.shape[0], &sites[0,0], sites.shape[0])

def gradient_descent_func(
        float[:,:,:,::1] linesegs not None, 
        float[:,:,::1] sites not None,
        float[::1] perimeter not None, 
        float jiggle):
    gradient_descent(&linesegs[0,0,0,0], &sites[0,0,0], &perimeter[0], jiggle, sites.shape[1], linesegs.shape[1]*linesegs.shape[2], perimeter.shape[0])
