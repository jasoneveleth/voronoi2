#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gradient.h"
#include <stdio.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) >= (b) ? (a) : (b))

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
            float dist = sqrtf((dx * dx) + (dy * dy));

            repulsion_term += options.repel_coeff * (1 / dist);
        }
    }
    return perimeter + repulsion_term;
}

float
obj_function(point *sites, struct edgelist *edgelist, int nsites)
{
    if ((options.obj & REPULSION) && (options.obj & PERIMETER))
        return obj_perimeter_and_repel(sites, edgelist, nsites);
    else if (options.obj & PERIMETER) {
        (void)sites;  // unused
        (void)nsites; // unused
        return calc_perimeter(edgelist);
    } else {
        fprintf(stderr, "\n%s:%d:%s: fatal error, options/logic wrong\n",
                __FILE__, __LINE__, __func__);
        exit(1);
    }
}

void
update_sites(point *src, point *dest, point *grad, int nsites, float alpha)
{
    for (int i = 0; i < nsites; i++) {
        point delta = {-(alpha * grad[i].x), -(alpha * grad[i].y)};
        dest[i] = boundary_cond(src[i], delta);
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
    const point deltax = {jiggle, 0.0f};
    const point deltay = {0.0f, jiggle};
    point *local_sites = malloc((size_t)nsites * sizeof(point));
    memcpy(local_sites, old_sites, (size_t)nsites * sizeof(point));
    struct edgelist local_edgelist;
    // x
    local_sites[j] = boundary_cond(local_sites[j], deltax);
    init_edgelist(&local_edgelist);
    fortunes(local_sites, nsites, &local_edgelist);
    float curr_obj = obj_function(local_sites, &local_edgelist, nsites);
    gradient[j].x = (curr_obj - prev_objective) / jiggle;
    free_edgelist(&local_edgelist);
    // reset for y
    local_sites[j] = old_sites[j];
    // y
    local_sites[j] = boundary_cond(local_sites[j], deltay);
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
    // we treat site.x and site.y as separate variables (x_k is a vector,
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

static void
calc_earth_mover(size_t nbins, int *earthmover, int *edgehist, int i)
{
    if (i > 0) {
        int *old_hist = &edgehist[(i - 1) * (int)nbins];
        int *new_hist = &edgehist[i * (int)nbins];
        int total = 0;
        int ith = 0;
        for (int j = 1; j < (int)nbins; j++) {
            ith = old_hist[j] + ith - new_hist[j];
            total += abs(ith);
        }
        earthmover[i] = total;
    } else {
        earthmover[i] = 0;
    }
}

static inline void
calc_stats(struct edgelist *edgelist,
           point *sites,
           float *perimeter,
           float *objective_function,
           float *char_max_length,
           float *char_min_length,
           int *edgehist,
           int nsites)
{
    *perimeter = calc_perimeter(edgelist);
    *objective_function = obj_function(sites, edgelist, nsites);
    calc_edge_length(edgelist, char_max_length, char_min_length, edgehist,
                     (size_t)nsites);
}

void
simple_descent(struct arrays numpy_arrs,
               const float jiggle,
               int nsites,
               const int pts_per_trial)
{
    // unpack numpy arrays
    point *linesegs = (point *)numpy_arrs.linesegs_to_be_cast;
    point *sites = (point *)numpy_arrs.sites_to_be_cast;
    float *perimeter = numpy_arrs.perimeter;
    float *obj_func_vals = numpy_arrs.objective_function;
    float *char_max_length = numpy_arrs.char_max_length;
    float *char_min_length = numpy_arrs.char_min_length;
    int *edgehist = numpy_arrs.edgehist;
    int *earthmover = numpy_arrs.earthmover;

    point *gradient = malloc((size_t)nsites * sizeof(point));
    for (int i = 0; i < (int)options.ntrials; i++) {
        if (i > 0) { // skip this the first time
            float prev_objective = obj_func_vals[i - 1];
            point *old_sites_ptr = &sites[(i - 1) * nsites];
            // PARALLEL
            for (int j = 0; j < nsites; j++)
                gradient_method(j, nsites, old_sites_ptr, gradient, jiggle,
                                prev_objective);
            update_sites(old_sites_ptr, &sites[i * nsites], gradient, nsites,
                         options.alpha);
        }

        struct edgelist edgelist;
        init_edgelist(&edgelist);
        fortunes(&sites[i * nsites], nsites, &edgelist);
        copy_edges(&edgelist, &linesegs[i * pts_per_trial]);
        calc_stats(&edgelist, &sites[i * nsites], &perimeter[i],
                   &obj_func_vals[i], &char_max_length[i], &char_min_length[i],
                   &edgehist[i * (int)(1.4143f * (float)nsites)], nsites);
        // HARDCODE
        calc_earth_mover((size_t)((float)nsites * 1.4143f), earthmover,
                         edgehist, i);
        free_edgelist(&edgelist);
    }
    free(gradient);
}

void
barziilai_borwein(struct arrays numpy_arrs,
                  const float jiggle,
                  int nsites,
                  const int pts_per_trial)
{
    // unpack numpy arrays
    point *linesegs = (point *)numpy_arrs.linesegs_to_be_cast;
    point *sites = (point *)numpy_arrs.sites_to_be_cast;
    float *perimeter = numpy_arrs.perimeter;
    float *obj_func_vals = numpy_arrs.objective_function;
    float *char_max_length = numpy_arrs.char_max_length;
    float *char_min_length = numpy_arrs.char_min_length;
    int *edgehist = numpy_arrs.edgehist;
    int *earthmover = numpy_arrs.earthmover;

    point *g_k = malloc((size_t)nsites * sizeof(point));
    point *g_k1 = malloc((size_t)nsites * sizeof(point));
    for (int i = 0; i < (int)options.ntrials; i++) {
        if (i > 0) { // skip this the first time
            float prev_objective = obj_func_vals[i - 1];
            point *old_sites_ptr = &sites[(i - 1) * nsites];
            // PARALLEL
            for (int j = 0; j < nsites; j++)
                gradient_method(j, nsites, old_sites_ptr, g_k, jiggle,
                                prev_objective);

            float alpha;
            if (i == 1) {
                alpha = options.alpha;
            } else {
                point *x_k1 = &sites[(i - 2) * nsites];
                point *x_k = &sites[(i - 1) * nsites];
                alpha = bb_formula(x_k1, x_k, g_k1, g_k, nsites);
                alpha = max(0, alpha);     // HARDCODE
                alpha = min(alpha, 1e-2f); // HARDCODE
            }

            update_sites(old_sites_ptr, &sites[i * nsites], g_k, nsites, alpha);
            point *tmp = g_k1;
            g_k1 = g_k;
            g_k = tmp;
        } // <- skipped the first time

        struct edgelist edgelist;
        init_edgelist(&edgelist);
        fortunes(&sites[i * nsites], nsites, &edgelist);
        copy_edges(&edgelist, &linesegs[i * pts_per_trial]);
        calc_stats(&edgelist, &sites[i * nsites], &perimeter[i],
                   &obj_func_vals[i], &char_max_length[i], &char_min_length[i],
                   &edgehist[i * (int)(1.4143f * (float)nsites)], nsites);
        // HARDCODE
        calc_earth_mover((size_t)((float)nsites * 1.4143f), earthmover,
                         edgehist, i);
        free_edgelist(&edgelist);
    }
    free(g_k);
    free(g_k1);
}
