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

typedef struct dpoint dpoint;
struct dpoint {
    double x;
    double y;
};

point intersect_parabolas(float, point *);
point __attribute__((const)) circleBottom(point, point, point);
dpoint __attribute__((const)) circle_center(point, point, point);
point __attribute__((const)) circle_centerf(point, point, point);
point boundary_cond(point, point);
float __attribute__((const)) frac(float);

#endif
