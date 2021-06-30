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
};

float obj_function(point *, struct edgelist *, int);
void update_sites(point *, point *, point *, int, float);
void gradient_method(const int,
                     const int,
                     const point *const,
                     point *,
                     const float,
                     const float);
float bb_formula(point *, point *, point *, point *, int);
void gradient_descent(struct arrays, const float, int, const int);

#endif
