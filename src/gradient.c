#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gradient.h"
#include <stdio.h>

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

void
gradient_method(const int j,
                const int nsites,
                const point *const old_sites,
                point *gradient,
                const float jiggle,
                const float prev_objective)
{
    if (options.gradient == FINITE_DIFFERENCE) {
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

static void *
wrapper(void *args_to_cast)
{
    struct pthread_args *args = (struct pthread_args *)args_to_cast;
    for (int i = args->start; i < args->end; i++) {
        gradient_method(i, args->nsites, args->old_sites, args->gradient,
                        args->jiggle, args->prev_objective);
    }
    return args_to_cast;
}

void
gradient_descent(struct arrays arr,
                 const float jiggle,
                 int nsites,
                 const int pts_per_trial)
{
    point *g_k = NULL;
    point *g_k1 = NULL;
    g_k = malloc((size_t)nsites * sizeof(point));
    if (options.descent == BARZILAI) {
        g_k1 = malloc((size_t)nsites * sizeof(point));
    }
    pthread_t *thr = (pthread_t *)malloc(NTHREADS * sizeof(pthread_t));
    for (int i = 0; i < (int)options.ntrials; i++) {
        if (i > 0) { // skip this the first time
            float prev_objective = arr.objective_function[i - 1];
            point *old_sites_ptr = &arr.sites[(i - 1) * nsites];
            // {{{ PARALLEL
            for (int j = 0; j < NTHREADS; j++) {
                struct pthread_args *thread_args =
                    malloc(sizeof(struct pthread_args));
                // we cast to work with const
                thread_args->start = j * nsites / NTHREADS;
                thread_args->end = (j + 1) * nsites / NTHREADS;
                thread_args->nsites = nsites;
                thread_args->old_sites = old_sites_ptr;
                thread_args->gradient = g_k;
                thread_args->jiggle = jiggle;
                thread_args->prev_objective = prev_objective;
                pthread_create(&thr[j], NULL, wrapper, thread_args);
            }
            for (int j = 0; j < NTHREADS; j++) {
                void *args = NULL;
                pthread_join(thr[j], &args);
                free(args);
            }
            // PARALLEL }}}
            float alpha;
            if (options.descent == BARZILAI) {
                if (i == 1) {
                    alpha = options.alpha;
                } else {
                    point *x_k1 = &arr.sites[(i - 2) * nsites];
                    point *x_k = &arr.sites[(i - 1) * nsites];
                    alpha = bb_formula(x_k1, x_k, g_k1, g_k, nsites);
                }
            } else {
                alpha = options.alpha;
            }
            arr.alpha[i] = alpha;
            update_sites(old_sites_ptr, &arr.sites[i * nsites], g_k, nsites,
                         alpha);
            if (options.descent == BARZILAI) {
                point *tmp = g_k1;
                g_k1 = g_k;
                g_k = tmp;
            }
        } // <- skipped the first time

        struct edgelist edgelist;
        init_edgelist(&edgelist);
        fortunes(&arr.sites[i * nsites], nsites, &edgelist);
        copy_edges(&edgelist, &arr.linesegs[i * pts_per_trial]);
        calc_stats(&edgelist, &arr.sites[i * nsites], &arr.perimeter[i],
                   &arr.objective_function[i], &arr.char_max_length[i],
                   &arr.char_min_length[i],
                   &arr.edgehist[i * (int)(1.4143f * (float)nsites)], nsites);
        // HARDCODE
        calc_earth_mover((size_t)((float)nsites * 1.4143f), arr.earthmover,
                         arr.edgehist, i);
        free_edgelist(&edgelist);
    }
    free(thr);
    free(g_k);
    free(g_k1);
}
