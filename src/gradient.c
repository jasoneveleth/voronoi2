#include <stdarg.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include "gradient.h"

static float
calc_objective(point *sites, struct edgelist *edgelist, int nsites)
{
    assert(options.obj < NOBJTYPES + 1); // validate, +1 bc sets start at 1
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
    }
    return -1.0f;
}

static inline void
bound_vec(float *const a, const size_t len)
{
    if (options.boundary == TORUS) {
        for (size_t i = 0; i < len; i++) {
            a[i] = frac(a[i]);
            if (a[i] < 0) a[i] = 1 + a[i];
        }
    } else if (options.boundary == BOUNCE) {
        for (size_t i = 0; i < len; i++) {
            a[i] = fmodf(a[i], 2.0f);
            if (a[i] < 0) { a[i] = -a[i]; }
            if (a[i] > 1) { a[i] = 2 - a[i]; }
        }
    }
}

static void
update_sites(point *src, point *dest, point *grad, int nsites, float alpha)
{
    for (int i = 0; i < nsites; i++) {
        point delta = {-(alpha * grad[i].x), -(alpha * grad[i].y)};
        dest[i] = boundary_cond(src[i], delta);
    }
}

static float
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
    //       (x_k - x_{k-1}) dot (x_k - x_{k-1})             s_{k-1} dot s_{k-1}
    // res = -----------------------------------  OR   res = -------------------
    //       (x_k - x_{k-1}) dot (g_k - g_{k-1})             s_{k-1} dot y_{k-1}
    //

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
calc_stats(
    point *sites, point *linesegs, float *perimeter, float *obj, int nsites)
{
    struct edgelist edgelist;
    init_edgelist(&edgelist);
    fortunes(sites, nsites, &edgelist);
    *obj = calc_objective(sites, &edgelist, nsites);
    *perimeter = calc_perimeter(&edgelist);
    copy_edges(&edgelist, linesegs);
    free_edgelist(&edgelist);
}

static inline float
objective_function(point *sites, int nsites)
{
    struct edgelist edgelist;
    init_edgelist(&edgelist);
    fortunes(sites, nsites, &edgelist);
    float obj = calc_objective(sites, &edgelist, nsites);
    free_edgelist(&edgelist);
    return obj;
}

static void *
calc_gradient(void *args_to_cast)
{
    struct pthread_args *args = (struct pthread_args *)args_to_cast;
    if (options.gradient != FINITE_DIFFERENCE) {
        RAISE("%s\n", "unknown gradient method");
    }
    for (int i = args->start; i < args->end; i++) {
        const point deltax = {args->jiggle, 0.0f};
        const point deltay = {0.0f, args->jiggle};
        size_t sites_size = (size_t)args->nsites * sizeof(point);
        point *local_sites = malloc(sites_size);
        memcpy(local_sites, args->old_sites, sites_size);
        // x
        local_sites[i] = boundary_cond(local_sites[i], deltax);
        float curr_obj = objective_function(local_sites, args->nsites);
        args->gradient[i].x = (curr_obj - args->prev_objective) / args->jiggle;
        // reset for y
        local_sites[i] = args->old_sites[i];
        // y
        local_sites[i] = boundary_cond(local_sites[i], deltay);
        curr_obj = objective_function(local_sites, args->nsites);
        args->gradient[i].y = (curr_obj - args->prev_objective) / args->jiggle;

        free(local_sites);
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
parallel_grad(point *grad,
              point *old_sites_ptr,
              size_t nsites,
              float prev_objective)
{
    pthread_t thr[NTHREADS];
    for (int j = 0; j < NTHREADS; j++) {
        struct pthread_args *thread_args = malloc(sizeof(struct pthread_args));
        thread_args->start = j * (int)nsites / NTHREADS;
        thread_args->end = (j + 1) * (int)nsites / NTHREADS;
        thread_args->nsites = (int)nsites;
        thread_args->old_sites = old_sites_ptr;
        thread_args->gradient = grad;
        thread_args->jiggle = options.jiggle;
        thread_args->prev_objective = prev_objective;
        pthread_create(&thr[j], NULL, calc_gradient, thread_args);
    }
    for (int j = 0; j < NTHREADS; j++) {
        void *args = NULL;
        pthread_join(thr[j], &args);
        free(args);
    }
}

static void
barzilai(int i, struct arrays arr, int nsites, point *g[])
{
    static float max_alpha = 1e-6f;
    static float min_alpha = 2e-2f;
    if (i == 0) return;
    point *r_i = g[0], *r_im1 = g[1];
    point *x_km1 = &arr.sites[(i - 2) * nsites];
    point *x_k = &arr.sites[(i - 1) * nsites];
    point *x_kp1 = &arr.sites[i * nsites];
    const float prev_obj = arr.objective_function[i - 1];

    parallel_grad(r_i, x_k, (size_t)nsites, prev_obj);

    if (i == 1) {
        arr.alpha[i] = 1e-8f;
    } else {
        arr.alpha[i] = bb_formula(x_km1, x_k, r_im1, r_i, nsites);
        arr.alpha[i] = arr.alpha[i] < max_alpha ? max_alpha : arr.alpha[i];
        arr.alpha[i] = arr.alpha[i] < min_alpha ? arr.alpha[i] : min_alpha;
    }

    // x_kp1 = x_k + a * r_i
    update_sites(x_k, x_kp1, r_i, nsites, arr.alpha[i]);

    g[0] = r_im1;
    g[1] = r_i;
}

static float
polakribiere(float *r_im1, float *r_i, size_t len)
{
    // https://en.wikipedia.org/wiki/Nonlinear_conjugate_gradient_method
    //       r_i^T (r_i - r_{i-1})
    // res = ---------------------
    //          r_{i-1}^T r_{i-1}

    double numerator = 0;
    double denominator = 0;
    for (size_t i = 0; i < len; i++) {
        double r_i_ele = (double)r_i[i], r_im1_ele = (double)r_im1[i];
        numerator += r_i_ele * (r_i_ele - r_im1_ele);
        denominator += r_im1_ele * r_im1_ele;
    }
    return (float)(numerator / denominator);
}

static double
dot(const float *const a, const float *const b, const size_t len)
{
    double res = 0;
    for (size_t i = 0; i < len; i++) { res += (double)a[i] * (double)b[i]; }
    return res;
}

static inline void
add(float *const a, const float *const b, const size_t len)
{
    for (size_t i = 0; i < len; i++) { a[i] += b[i]; }
}

static inline void
scale(float *const a, size_t len, const float c)
{
    for (size_t i = 0; i < len; i++) { a[i] *= c; }
}

static inline void
copy(float *const a, const float *const b, const size_t len)
{
    for (size_t i = 0; i < len; i++) { a[i] = b[i]; }
}

static inline void
print_vec(const float *const a, const size_t len, FILE *file)
{
    for (size_t i = 0; i < len; i++) { fprintf(file, "%f\n", (double)a[i]); }
}

//            obj        prev_obj               old_grad
//        v-----------v    v--v                v---------v
// >>     f (x + a * d) <= f(x) + c1 * a * d^T \nabla f(x)
//          ^---------^                    ^-------------^
//             new_x                        old_dot_prod
//
//
//              new_dot_prod                    old_grad
//        v----------------------v             v---------v
// >>     -d^T \nabla f (x + a * d) <= -c2 d^T \nabla f(x)
//                      ^---------^        ^-------------^
//                          new_x            old_dot_prod
//             ^------------------^
//                    new_grad
static void
linesearch(
    point *x_k_pt, point *d_pt, size_t nsites, float *alpha, float prev_obj_f)
{
    size_t len = nsites * 2;
    float *x_k = (float *)x_k_pt;
    const float *const d = (float *)d_pt;

    float *old_grad = malloc(len * sizeof(float));
    float *new_grad = malloc(len * sizeof(float));
    float *new_x = malloc(len * sizeof(float));

    parallel_grad((point *)old_grad, x_k_pt, nsites, prev_obj_f);
    double old_dot_prod = dot(d, old_grad, len);
    assert(old_dot_prod < 0); // descend direction

    // HARDCODE
    static const double c1 = 1e-4;
    static const double c2 = 0.1;
    static const double backtrack = 0.7;
    static const double limit = 1e-10; // 3e-5 for 1000pts
    double a = 1e-2;
    while (1) {
        a *= backtrack;
        if (a < limit) break; // too small HARDCODE

        // new_x = x_k + a * d
        copy(new_x, d, len);
        scale(new_x, len, (float)a);
        add(new_x, x_k, len);
        bound_vec(new_x, len);

        double prev_obj = (double)prev_obj_f;
        double obj = (double)objective_function((point *)new_x, (int)nsites);

        bool wolfe_cond1 = obj <= prev_obj + a * c1 * old_dot_prod;
        if (!wolfe_cond1) continue;

        parallel_grad((point *)new_grad, (point *)new_x, nsites, prev_obj_f);
        double new_dot_prod = dot(d, new_grad, len);
        bool wolfe_cond2 = -new_dot_prod <= -c2 * old_dot_prod;
        if (!wolfe_cond2) continue;

        break;
    }
    *alpha = (float)a;
    fprintf(stderr, a < limit ? "linesearch got too small\n" : "");

    free(new_x);
    free(new_grad);
    free(old_grad);
}

static void
conjugate(int i, struct arrays arr, int nsites, point *g[])
{
    float *r_im1 = (float *)g[0], *r_i = (float *)g[1];
    float *d_im1 = (float *)g[2], *d_i = (float *)g[3];
    float *x_im1 = (float *)&arr.sites[(i - 1) * nsites];
    float *x_i = (float *)&arr.sites[i * nsites];
    const float prev_obj = i > 0 ? arr.objective_function[i - 1]
                                 : objective_function((point *)x_i, nsites);
    size_t len = (size_t)nsites * 2;

    if (i == 0) {
        arr.alpha[i] = options.alpha;
        parallel_grad((point *)d_i, (point *)x_i, (size_t)nsites, prev_obj);
        scale(d_i, len, -1.0f);
        copy(r_i, d_i, len);
    } else {
        // a = argmin_a[f(x_im1 - a * d_im1)]
        linesearch((point *)x_im1, (point *)d_im1, (size_t)nsites,
                   &arr.alpha[i], prev_obj);

        // x_i = x_im1 - a * d_im1
        copy(x_i, d_im1, len);
        scale(x_i, len, -arr.alpha[i]);
        add(x_i, x_im1, len);
        bound_vec(x_i, len);

        // r_i = - \nabla f(x_i)
        parallel_grad((point *)r_i, (point *)x_i, (size_t)nsites, prev_obj);
        scale(r_i, len, -1.0f);

        // b = max(polak_ribiere, 0)
        float beta = polakribiere(r_im1, r_i, len);
        beta = beta > 0 ? beta : 0;

        // d_i = r_i + b * d_im1
        copy(d_i, d_im1, len);
        scale(d_i, len, beta);
        add(d_i, r_i, len);
    }

    g[0] = (point *)r_i;
    g[1] = (point *)r_im1;
    g[2] = (point *)d_i;
    g[3] = (point *)d_im1;
}

static void
constant_alpha(int i, struct arrays arr, int nsites, point *g[])
{
    if (i == 0) return;
    point *x_k1 = &arr.sites[(i - 1) * nsites];
    point *x_k = &arr.sites[i * nsites];
    point *r_i = g[0];
    const float prev_obj = arr.objective_function[i - 1];

    parallel_grad(r_i, x_k1, (size_t)nsites, prev_obj);
    arr.alpha[i] = options.alpha;

    size_t len = (size_t)nsites * 2;
    copy((float *)x_k, (float *)r_i, len);
    scale((float *)x_k, len, -arr.alpha[i]);
    add((float *)x_k, (float *)x_k1, len);
    bound_vec((float *)x_k, len);
}

static void
steepest_descent(int i, struct arrays arr, int nsites, point *g[])
{
    if (i == 0) return;
    point *x_k1 = &arr.sites[(i - 1) * nsites];
    point *x_k = &arr.sites[i * nsites];
    point *r_i = g[0];
    const float prev_obj = arr.objective_function[i - 1];
    size_t len = (size_t)nsites * 2;

    parallel_grad(r_i, x_k1, (size_t)nsites, prev_obj);
    scale((float *)r_i, len, -1);

    // a = argmin_a[f(x_k1 + a * r_i)]
    linesearch(x_k1, r_i, (size_t)nsites, &arr.alpha[i], prev_obj);

    // x_k = x_k1 + a * r_i
    copy((float *)x_k, (float *)r_i, len);
    scale((float *)x_k, len, arr.alpha[i]);
    add((float *)x_k, (float *)x_k1, len);
    bound_vec((float *)x_k, len);
}

void
gradient_descent(struct arrays arr, int nsites, const int pts_per_trial)
{
    // preallocate vectors in case they are needed by a descent method
    point *g[NGRADIENT_VECS] = {NULL, NULL, NULL, NULL};
    size_t grad_size = (size_t)nsites * sizeof(point);
    for (int i = 0; i < NGRADIENT_VECS; i++) g[i] = malloc(grad_size);

    for (int i = 0; i < (int)options.ntrials; i++) {
        myprint("\rdescent trial: %d ", i);
        assert(options.descent < NDESCENTTYPES); // validate enum
        static const descent_func jmp_table[NDESCENTTYPES] = {
            constant_alpha, barzilai, conjugate, steepest_descent};
        jmp_table[options.descent](i, arr, nsites, g);
        calc_stats(&arr.sites[i * nsites], &arr.linesegs[i * pts_per_trial],
                   &arr.perimeter[i], &arr.objective_function[i], nsites);
    }
    myprint("\r");
    for (int i = 0; i < NGRADIENT_VECS; i++) free(g[i]);
}
