# import both numpy and the Cython declarations for numpy
import numpy as np
cimport numpy as np

# if you want to use the Numpy-C-API from Cython
# (not strictly necessary for this example)
np.import_array()

# cdefine the signature of our c function
cdef extern from "src/cython_stuff.h":
    void simple_diagram(float *edges, int size, float *sites, int nsites)
    cdef struct arrays:
        float *linesegs_to_be_cast
        float *sites_to_be_cast
        float *perimeter
        float *objective_function
    void gradient_descent(arrays arrs, float jiggle, int sites, int pts_per_trial, int trials)

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
        float[::1] objective_functions not None, 
        float jiggle):
    cdef arrays arrs
    arrs.linesegs_to_be_cast = &linesegs[0,0,0,0]
    arrs.sites_to_be_cast = &sites[0,0,0]
    arrs.perimeter = &perimeter[0]
    arrs.objective_function = &objective_functions[0]
    gradient_descent(arrs, jiggle, sites.shape[1], linesegs.shape[1]*linesegs.shape[2], perimeter.shape[0])
