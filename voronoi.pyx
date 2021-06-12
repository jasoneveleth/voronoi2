# import both numpy and the Cython declarations for numpy
import numpy as np
cimport numpy as np

# if you want to use the Numpy-C-API from Cython
np.import_array()

# cdefine the signature of our c function
cdef extern from "src/cython_stuff.h":
    void simple_diagram(float *edges, int size, float *sites, int nsites)
    cdef struct arrays:
        float *linesegs_to_be_cast
        float *sites_to_be_cast
        float *perimeter
        float *objective_function
        float *char_max_length
        float *char_min_length
    void gradient_descent(arrays arrs, int sites, int pts_per_trial)
    void init_options(const char *filepath, unsigned char obj, unsigned char descent, unsigned char gradient, unsigned char boundary, float alpha, float repel_coeff, int ntrials, float jiggle);

# wrapper function. 
# ::1 in the last axis makes 'arr' contiguous in memory, and the rest of the type
# definition make 'arr' a memory view. To call the c function we need a pointer
# though, which we can get by making a point to the first element of the memory
# view. The 'not None' tells Cython that I never intend to pass None, so it can
# turn off some checks for speed
def simple_diagram_func(float[:,:,::1] arr not None, float[:,::1] sites not None):
    simple_diagram(&arr[0,0,0], arr.shape[0], &sites[0,0], sites.shape[0])

def gradient_descent_func(
        args,
        float[:,:,:,::1] linesegs not None, 
        float[:,:,::1] sites not None,
        float[::1] perimeter not None, 
        float[::1] objective_functions not None, 
        float[::1] char_max_length not None, 
        float[::1] char_min_length not None):
    cdef arrays arrs
    arrs.linesegs_to_be_cast = &linesegs[0,0,0,0]
    arrs.sites_to_be_cast = &sites[0,0,0]
    arrs.perimeter = &perimeter[0]
    arrs.objective_function = &objective_functions[0]
    arrs.char_max_length = &char_max_length[0]
    arrs.char_min_length = &char_min_length[0]

    # <***> casts just like C's (***)
    init_options(args.file.encode('utf-8'), args.objective, args.descent, args.gradient, args.boundary, args.alpha, args.repel_coeff, args.ntrials, args.jiggle);
    # init_options("input", 1, 0, 0, 1, 3e-3, 1e-4, perimeter.shape[0]);
    gradient_descent(arrs, sites.shape[1], linesegs.shape[1]*linesegs.shape[2])

