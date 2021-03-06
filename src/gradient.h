#ifndef GRADIENT_H
#define GRADIENT_H

#include "fortunes.h"

#define NGRADIENT_VECS 4

struct arrays {
    point *linesegs;
    point *sites;
    float *perimeter;
    float *objective_function;
    float *char_max_length;
    float *char_min_length;
    float *edgehist;
    float *earthmover;
    float *alpha;
};

struct pthread_args {
    int start;
    int end;
    int nsites;
    float jiggle;
    const point *old_sites;
    point *gradient;
    float prev_objective;
    const char padding[4];
};

typedef void (*descent_func)(int, struct arrays, int, point **);

void gradient_descent(struct arrays, int, const int);

#endif
