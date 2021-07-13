#ifndef GRADIENT_H
#define GRADIENT_H

#include "fortunes.h"

struct arrays {
    point *linesegs;
    point *sites;
    float *perimeter;
    float *objective_function;
    float *char_max_length;
    float *char_min_length;
    int *edgehist;
    int *earthmover;
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

typedef void (*descent_func)(int, struct arrays, int, pthread_t *, point **);

void gradient_descent(struct arrays, int, const int);

#endif
