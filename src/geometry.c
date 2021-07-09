#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include "geometry.h"

point
boundary_cond(point p, point delta)
{
    if (options.boundary == TORUS) {
        p.x += delta.x;
        p.y += delta.y;
        p.x = frac(p.x);
        if (p.x < 0) p.x = 1 + p.x;
        p.y = frac(p.y);
        if (p.y < 0) p.y = 1 + p.y;
    } else if (options.boundary == BOUNCE) {
        p.x += delta.x;
        p.y += delta.y;
        while (p.x > 1 || p.x < 0 || p.y > 1 || p.y < 0) {
            if (p.y < 0) { p.y = -p.y; }
            if (p.y > 1) { p.y = 1 - (p.y - 1); }
            if (p.x < 0) { p.x = -p.x; }
            if (p.x > 1) { p.x = 1 - (p.x - 1); }
        }
    } else {
        fprintf(stderr, "\n%s:%d:%s: fatal error, options/logic wrong\n",
                __FILE__, __LINE__, __func__);
        exit(1);
    }
    return p;
}

float
frac(float x)
{
    float useless_required_ptr;
    return modff(x, &useless_required_ptr);
}

static inline void
quadraticFormula(double a, double b, double c, double *smaller, double *larger)
{
    // https://en.wikipedia.org/wiki/Loss_of_significance#A_better_algorithm
    double sgn_b = copysign(1.0, b);
    double des = b * b - 4 * a * c;
    if (des < 0) {
        RAISE("Floating point exception sqrt(%f)\n", des);
        des = 1e-15;
    }
    double x1 = (-b - sgn_b * sqrt(des)) / (2 * a);
    double x2 = c / (a * x1);
    *smaller = x1 > x2 ? x2 : x1;
    *larger = x1 > x2 ? x1 : x2;
}

dpoint
circle_center(point af, point bf, point cf)
{
    const dpoint a = {(double)af.x, (double)af.y};
    const dpoint b = {(double)bf.x, (double)bf.y};
    const dpoint c = {(double)cf.x, (double)cf.y};
    double d =
        2.0 * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));
    if (fabs(d) < 1e-19) {
        printf("a: (%f, %f)\nb: (%f, %f)\nc: (%f, %f)\n", a.x, a.y, b.x, b.y,
               c.x, c.y);
    }
    double x = (1.0 / d)
               * ((a.x * a.x + a.y * a.y) * (b.y - c.y)
                  + (b.x * b.x + b.y * b.y) * (c.y - a.y)
                  + (c.x * c.x + c.y * c.y) * (a.y - b.y));
    double y = (1.0 / d)
               * ((a.x * a.x + a.y * a.y) * (c.x - b.x)
                  + (b.x * b.x + b.y * b.y) * (a.x - c.x)
                  + (c.x * c.x + c.y * c.y) * (b.x - a.x));
    dpoint p = {x, y};
    return p;
}

point
circle_centerf(point a, point b, point c)
{
    dpoint p = circle_center(a, b, c);
    point pf = {(float)p.x, (float)p.y};
    return pf;
}

point
circleBottom(point a, point b, point c)
{
    dpoint p = circle_center(a, b, c);
    double x = (double)a.x - (double)p.x;
    double y = (double)a.y - (double)p.y;
    double r = sqrt(x * x + y * y);
    point pf = {(float)p.x, (float)(p.y - r)};
    return pf;
}

point
intersect_parabolas(float sweepline, point *parabolas)
{
    // parabola on the left of bp
    const dpoint p1 = {(double)parabolas[0].x, (double)parabolas[0].y};
    // parabola on the right of bp
    const dpoint p2 = {(double)parabolas[1].x, (double)parabolas[1].y};
    const double l = (double)sweepline;

    // HARDCODE
    bool sweepline_and_point_same_y = p1.y - l < 1e-8 || p2.y - l < 1e-8;
    if (sweepline_and_point_same_y) {
        dpoint lower = p1.y < p2.y ? p1 : p2;
        dpoint higher = p1.y < p2.y ? p2 : p1;
        // (x-h)^2 = 4p(y-k)
        const double x = lower.x;
        const double h = higher.x;
        const double k = (higher.y + l) / 2.0;
        const double p = 2.0 * (higher.y - l);
        const double y = (x - h) * (x - h) / (4 * p) + k;
        point intersection = {(float)x, (float)y};
        return intersection;
    }

    const double a = 1.0 / (2.0 * (p1.y - l)) - 1.0 / (2.0 * (p2.y - l));
    const double b = (p2.x) / (p2.y - l) - (p1.x) / (p1.y - l);
    const double c = (p1.x * p1.x + p1.y * p1.y - l * l) / (2.0 * (p1.y - l))
                     - (p2.x * p2.x + p2.y * p2.y - l * l) / (2.0 * (p2.y - l));

    // HARDCODE
    bool mutliple_points_have_same_y_value = fabs(p1.y - p2.y) < 1e-8;
    if (mutliple_points_have_same_y_value) {
        const double x = -c / b;
        const double y =
            (1.0 / (2.0 * (p1.y - l))
             * (x * x - 2.0 * p1.x * x + p1.x * p1.x + p1.y * p1.y - l * l));
        point p = {(float)x, (float)y};
        return p;
    }

    double x1, x2;
    quadraticFormula(a, b, c, &x1, &x2);
    assert(x1 <= x2);
    const double y1 =
        (1.0 / (2.0 * (p1.y - l)))
        * (x1 * x1 - 2.0 * p1.x * x1 + p1.x * p1.x + p1.y * p1.y - l * l);
    const double y2 =
        (1.0 / (2.0 * (p1.y - l)))
        * (x2 * x2 - 2.0 * p1.x * x2 + p1.x * p1.x + p1.y * p1.y - l * l);
    point left = {(float)x1, (float)y1};
    point right = {(float)x2, (float)y2};

    bool parabola_order_is_old_then_new = p1.y > p2.y;
    if (parabola_order_is_old_then_new) {
        return left;
    } else {
        return right;
    }
}
