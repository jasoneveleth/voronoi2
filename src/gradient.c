#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gradient.h"
#include <stdio.h>
#include "fortunes.h"

static inline float
obj_perimeter_and_repel(point *sites, struct edgelist *edgelist, int nsites)
{
    float perimeter = calc_perimeter(edgelist);
    float repulsion_term = 0;
    for (int i = 0; i < nsites; i++) {
        for (int j = 0; j < nsites; j++) {
            if (i == j) continue;
            float dx = sites[i].x - sites[j].x;
            float dy = sites[i].y - sites[j].y;
            float dist = fsqrt((dx * dx) + (dy * dy));

            const float coeff = 1e-4f; // MMM
            repulsion_term += coeff * (1 / dist);
        }
    }
    return perimeter + repulsion_term;
}

float
obj_function(point *sites, struct edgelist *edgelist, int nsites)
{
#ifdef REPEL
    return obj_perimeter_and_repel(sites, edgelist, nsites);
#else
    (void)sites;  // unused
    (void)nsites; // unused
    return calc_perimeter(edgelist);
#endif
}

void
update_sites(point *src, point *dest, point *grad, int nsites, float alpha)
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
    free_edgelist(&local_edgelist);
    // reset for y
    local_sites[j].x = old_sites[j].x;
    // y
    local_sites[j].y = frac(local_sites[j].y + jiggle);
    init_edgelist(&local_edgelist);
    fortunes(local_sites, nsites, &local_edgelist);
    curr_obj = obj_function(local_sites, &local_edgelist, nsites);
    gradient[j].y = (curr_obj - prev_objective) / jiggle;
    free_edgelist(&local_edgelist);

    free(local_sites);
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

float
bb_formula(
    point *x_k1_pt, point *x_k_pt, point *g_k1_pt, point *g_k_pt, int nsites)
{
    // we tree site.x and site.y as separate variables (x_k is a vector,
    // unrelated to that, the name x_k for the vector is just to remain
    // consistent with the website detailing the formula (linked below)).
    float *x_k1 = (float *)x_k1_pt;
    float *x_k = (float *)x_k_pt;
    float *g_k1 = (float *)g_k1_pt;
    float *g_k = (float *)g_k_pt;
    // https://bicmr.pku.edu.cn/~wenzw/courses/WenyuSun_YaxiangYuan_BB.pdf
    //       (x_k - x_{k-1}) dot (x_k - x_{k-1})
    // res = -----------------------------------
    //       (x_k - x_{k-1}) dot (g_k - g_{k-1})
    //
    // AKA
    //
    //       s_{k-1} dot s_{k-1}
    // res = -------------------
    //       s_{k-1} dot y_{k-1}

    float numerator = 0;
    float denominator = 0;
    for (int i = 0; i < 2 * nsites; i++) {
        float s_k1 = x_k[i] - x_k1[i];
        float y_k1 = g_k[i] - g_k1[i];
        numerator += s_k1 * s_k1;
        denominator += s_k1 * y_k1;
    }
    return numerator / denominator;
}
