#ifndef GEOMETRY_H
#define GEOMETRY_H
#include "config.h"

extern struct options options;

typedef struct point point;
struct point {
    float x;
    float y;
};

point intersect_parabolas(float, point *);
point circleBottom(point, point, point);
point circle_center(point, point, point);
point boundary_cond(point, point);
float frac(float);

#endif
