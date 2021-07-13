#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include "gradient.h"

float
obj_function(point *sites, struct edgelist *edgelist, int nsites)
{
    if (options.obj < NOBJTYPES) { RAISE("%s\n", "options/logic wrong"); }
    if ((options.obj & REPULSION) && (options.obj & PERIMETER)) {
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
    } else if (options.obj & PERIMETER) {
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

    double numerator = 0;
    double denominator = 0;
    for (int i = 0; i < 2 * nsites; i++) {
        double s_k1 = (double)x_k[i] - (double)x_k1[i];
        double y_k1 = (double)g_k[i] - (double)g_k1[i];
        numerator += s_k1 * s_k1;
        denominator += s_k1 * y_k1;
    }
    return (float)(numerator / denominator);
}

static inline void
calc_stats(struct edgelist *edgelist,
           point *sites,
           float *perimeter,
           float *objective_function,
           int nsites)
{
    *perimeter = calc_perimeter(edgelist);
    *objective_function = obj_function(sites, edgelist, nsites);
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

static void
myprint(const char *format, ...)
{
    if (!options.silent) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        fflush(stdout);
        va_end(args);
    }
}

static void
parallel_grad(size_t nsites,
              pthread_t *thr,
              float prev_objective,
              point *old_sites_ptr,
              float jiggle,
              point *grad)
{
    for (int j = 0; j < NTHREADS; j++) {
        struct pthread_args *thread_args = malloc(sizeof(struct pthread_args));
        thread_args->start = j * (int)nsites / NTHREADS;
        thread_args->end = (j + 1) * (int)nsites / NTHREADS;
        thread_args->nsites = (int)nsites;
        thread_args->old_sites = old_sites_ptr;
        thread_args->gradient = grad;
        thread_args->jiggle = jiggle;
        thread_args->prev_objective = prev_objective;
        pthread_create(&thr[j], NULL, wrapper, thread_args);
    }
    for (int j = 0; j < NTHREADS; j++) {
        void *args = NULL;
        pthread_join(thr[j], &args);
        free(args);
    }
}

static void
barzilai(int i, struct arrays arr, int nsites, pthread_t *thr, point *g[])
{
    point *r_i = g[0], *r_im1 = g[1];
    point *x_km1 = &arr.sites[(i - 2) * nsites];
    point *x_k = &arr.sites[(i - 1) * nsites];
    point *x_kp1 = &arr.sites[i * nsites];

    parallel_grad((size_t)nsites, thr, arr.objective_function[i - 1], x_k,
                  options.jiggle, r_i);

    if (i == 1) {
        arr.alpha[i] = options.alpha;
    } else {
        arr.alpha[i] = bb_formula(x_km1, x_k, r_im1, r_i, nsites);
    }

    update_sites(x_k, x_kp1, r_i, nsites, arr.alpha[i]);

    g[0] = r_im1;
    g[1] = r_i;
}

static void
conjugate(int i, struct arrays arr, int nsites, pthread_t *thr, point *g[])
{
    // point *r_i = g[0], *r_im1 = g[1];
    // point *d_i = g[2], *d_im1 = g[3];
    // if (i == 1) {}
    (void)i;
    (void)arr;
    (void)nsites;
    (void)thr;
    (void)g;
}

static void
constant_alpha(int i, struct arrays arr, int nsites, pthread_t *thr, point *g[])
{
    point *x_k1 = &arr.sites[(i - 1) * nsites];
    point *x_k = &arr.sites[i * nsites];
    point *r_i = g[0];

    parallel_grad((size_t)nsites, thr, arr.objective_function[i - 1], x_k1,
                  options.jiggle, r_i);
    arr.alpha[i] = options.alpha;
    update_sites(x_k1, x_k, r_i, nsites, arr.alpha[i]);
}

void
gradient_descent(struct arrays arr, int nsites, const int pts_per_trial)
{
    point *g[4] = {NULL, NULL, NULL, NULL};
    g[0] = malloc((size_t)nsites * sizeof(point));
    g[1] = malloc((size_t)nsites * sizeof(point));
    g[2] = malloc((size_t)nsites * sizeof(point));
    g[3] = malloc((size_t)nsites * sizeof(point));
    pthread_t *thr = (pthread_t *)malloc(NTHREADS * sizeof(pthread_t));
    for (int i = 0; i < (int)options.ntrials; i++) {
        myprint("\rdescent trial: %d ", i);
        if (i > 0) {                                 // skip this the first time
            assert(options.descent < NDESCENTTYPES); // validate enum
            descent_func descent[NDESCENTTYPES] = {constant_alpha, barzilai,
                                                   conjugate};
            descent[options.descent](i, arr, nsites, thr, g);
        } // <- skipped the first time

        struct edgelist edgelist;
        init_edgelist(&edgelist);
        fortunes(&arr.sites[i * nsites], nsites, &edgelist);
        copy_edges(&edgelist, &arr.linesegs[i * pts_per_trial]);
        calc_stats(&edgelist, &arr.sites[i * nsites], &arr.perimeter[i],
                   &arr.objective_function[i], nsites);
        free_edgelist(&edgelist);
    }
    myprint("\r");
    free(thr);
    free(g[0]);
    free(g[1]);
    free(g[2]);
    free(g[3]);
}
