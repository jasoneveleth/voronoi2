#include "gradient.h"

float
obj_perimeter(__attribute__((unused)) point *sites,
              struct edgelist *edgelist,
              __attribute__((unused)) int nsites)
{
    return calc_perimeter(edgelist);
}

static inline float
repel_formula(const float x)
{
    const float coeff = (float)2e-9; // INTERFACE
    const float adjusted = (float)0.01 + x;
    return coeff * (1 / (adjusted * adjusted * adjusted * adjusted));
}

float
obj_perimeter_and_repel(point *sites, struct edgelist *edgelist, int nsites)
{
    float obj_function = calc_perimeter(edgelist);
    for (int i = 0; i < nsites; i++) {
        for (int j = 0; j < nsites; j++) {
            if (i == j) continue;
            float dx = sites[i].x - sites[j].x;
            float dy = sites[i].y - sites[j].y;
            float dist_squared = dx * dx + dy * dy;
            obj_function += repel_formula(dist_squared);
        }
    }
    return obj_function;
}

void
update_sites(point *src, point *dest, point *grad, int nsites)
{
    for (int i = 0; i < nsites; i++) {
        dest[i].x = frac(src[i].x - alpha * grad[i].x);
        if (dest[i].x < 0) dest[i].x = 1 + dest[i].x;
        dest[i].y = frac(src[i].y - alpha * grad[i].y);
        if (dest[i].y < 0) dest[i].y = 1 + dest[i].y;
    }
}
