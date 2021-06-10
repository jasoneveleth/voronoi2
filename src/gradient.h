#ifndef GRADIENT_H
#define GRADIENT_H

#include "edgelist.h"

float obj_function(point *, struct edgelist *, int);
void update_sites(point *, point *, point *, int, float);
void gradient_method(const int,
                     const int,
                     const point *const,
                     point *,
                     const float,
                     const float);
float bb_formula(point *, point *, point *, point *, int);

#endif
