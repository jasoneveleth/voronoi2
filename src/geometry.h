#ifndef GEOMETRY_H
#define GEOMETRY_H

typedef struct point point;
struct point {
    float x;
    float y;
};

point intersect_parabolas(float, point *);
float fsqrt(float);
point circleBottom(point, point, point);
point circle_center(point, point, point);

#endif
