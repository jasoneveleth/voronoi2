#ifndef GEOMETRY_H
#define GEOMETRY_H
#include "config.h"

#define RAISE(fmt, ...)                                               \
    do {                                                              \
        fprintf(stderr, "error %s:%d:%s(): " fmt, __FILE__, __LINE__, \
                __func__, __VA_ARGS__);                               \
    } while (0)

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
