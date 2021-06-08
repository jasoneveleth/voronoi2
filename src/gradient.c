#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gradient.h"
#include "fortunes.h"

float
obj_function(__attribute__((unused)) point *sites,
             struct edgelist *edgelist,
             __attribute__((unused)) int nsites)
{
    return calc_perimeter(edgelist);
}

// static inline float
// repel_formula(const float x)
// {
//     const float coeff = (float)2e-9; // INTERFACE
//     const float adjusted = (float)0.01 + x;
//     return coeff * (1 / (adjusted * adjusted * adjusted * adjusted));
// }

// static inline float
// obj_perimeter_and_repel(point *sites, struct edgelist *edgelist, int nsites)
// {
//     float obj_function = calc_perimeter(edgelist);
//     for (int i = 0; i < nsites; i++) {
//         for (int j = 0; j < nsites; j++) {
//             if (i == j) continue;
//             float dx = sites[i].x - sites[j].x;
//             float dy = sites[i].y - sites[j].y;
//             float dist_squared = dx * dx + dy * dy;
//             obj_function += repel_formula(dist_squared);
//         }
//     }
//     return obj_function;
// }

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

static inline void
finite_difference(const int j,
                  const int nsites,
                  const point *const old_sites,
                  point *gradient,
                  const float jiggle,
                  const float prev_objective)
{
    point *local_sites = malloc((size_t)nsites * sizeof(point));
    memcpy(local_sites, old_sites, (size_t)nsites * sizeof(point));
    struct edgelist local_edgelist;
    // x
    local_sites[j].x = frac(local_sites[j].x + jiggle);
    init_edgelist(&local_edgelist);
    fortunes(local_sites, nsites, &local_edgelist);
    float curr_obj = obj_function(local_sites, &local_edgelist, nsites);
    gradient[j].x = (curr_obj - prev_objective) / jiggle;
    local_sites[j].x = old_sites[j].x; // reset for y
    free_edgelist(&local_edgelist);    // reset for y
    // y
    local_sites[j].y = frac(local_sites[j].y + jiggle);
    init_edgelist(&local_edgelist);
    fortunes(local_sites, nsites, &local_edgelist);
    curr_obj = obj_function(local_sites, &local_edgelist, nsites);
    gradient[j].y = (curr_obj - prev_objective) / jiggle;

    free(local_sites);
    free_edgelist(&local_edgelist);
}

void
gradient_method(const int j,
                const int nsites,
                const point *const old_sites,
                point *gradient,
                const float jiggle,
                const float prev_objective)
{
    finite_difference(j, nsites, old_sites, gradient, jiggle, prev_objective);
}
