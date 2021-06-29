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
quadraticFormula(float a, float b, float c, float *smaller, float *larger)
{
    // https://en.wikipedia.org/wiki/Loss_of_significance#A_better_algorithm
    float sgn_b = copysignf(1.0f, b);
    float des = b * b - 4 * a * c;
    float x1 = (-b - sgn_b * sqrtf(des)) / (2 * a);
    float x2 = c / (a * x1);
    *smaller = x1 > x2 ? x2 : x1;
    *larger = x1 > x2 ? x1 : x2;
}

point
circle_center(point a, point b, point c)
{
    float d =
        2.0f * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));
    float x = (1.0f / d)
              * ((a.x * a.x + a.y * a.y) * (b.y - c.y)
                 + (b.x * b.x + b.y * b.y) * (c.y - a.y)
                 + (c.x * c.x + c.y * c.y) * (a.y - b.y));
    float y = (1.0f / d)
              * ((a.x * a.x + a.y * a.y) * (c.x - b.x)
                 + (b.x * b.x + b.y * b.y) * (a.x - c.x)
                 + (c.x * c.x + c.y * c.y) * (b.x - a.x));
    point p = {x, y};
    return p;
}

point
circleBottom(point a, point b, point c)
{
    point p = circle_center(a, b, c);
    float r = sqrtf((a.x - p.x) * (a.x - p.x) + (a.y - p.y) * (a.y - p.y));
    p.y -= r;
    return p;
}

point
intersect_parabolas(float sweepline, point *parabolas)
{
    const point p1 = parabolas[0]; // parabola on the left of bp
    const point p2 = parabolas[1]; // parabola on the right of bp
    const float l = sweepline;

    // HARDCODE
    bool sweepline_and_point_same_y = p1.y - l < 1e-6f || p2.y - l < 1e-6f;
    if (sweepline_and_point_same_y) {
        point lower = p1.y < p2.y ? p1 : p2;
        point higher = p1.y < p2.y ? p2 : p1;
        // (x-h)^2 = 4p(y-k)
        const float x = lower.x;
        const float h = higher.x;
        const float k = (higher.y + l) / 2.0f;
        const float p = 2.0f * (higher.y - l);
        const float y = (x - h) * (x - h) / (4 * p) + k;
        point intersection = {x, y};
        return intersection;
    }

    const float a = 1.0f / (2.0f * (p1.y - l)) - 1.0f / (2.0f * (p2.y - l));
    const float b = (p2.x) / (p2.y - l) - (p1.x) / (p1.y - l);
    const float c = (p1.x * p1.x + p1.y * p1.y - l * l) / (2.0f * (p1.y - l))
                    - (p2.x * p2.x + p2.y * p2.y - l * l) / (2.0f * (p2.y - l));

    // HARDCODE
    bool mutliple_points_have_same_y_value = fabsf(p1.y - p2.y) < 1e-6f;
    if (mutliple_points_have_same_y_value) {
        const float x = -c / b;
        const float y =
            (1.0f / (2.0f * (p1.y - l))
             * (x * x - 2.0f * p1.x * x + p1.x * p1.x + p1.y * p1.y - l * l));
        point p = {x, y};
        return p;
    }

    float x1, x2;
    quadraticFormula(a, b, c, &x1, &x2);
    assert(x1 <= x2);
    const float y1 =
        (1.0f / (2.0f * (p1.y - l)))
        * (x1 * x1 - 2.0f * p1.x * x1 + p1.x * p1.x + p1.y * p1.y - l * l);
    const float y2 =
        (1.0f / (2.0f * (p1.y - l)))
        * (x2 * x2 - 2.0f * p1.x * x2 + p1.x * p1.x + p1.y * p1.y - l * l);
    point left = {x1, y1};
    point right = {x2, y2};

    int parabola_order_is_old_then_new = p1.y > p2.y;
    if (parabola_order_is_old_then_new) {
        return left;
    } else {
        return right;
    }
}
