#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bintree.h"
#include "fortunes.h"
#include "heap.h"
#include "cython_stuff.h"

typedef float (*obj_func)(point *sites, struct edgelist *, int);
typedef void (*grad_func)(const int,
                          const int,
                          const point *const,
                          point *,
                          const float,
                          const float,
                          obj_func);

static const float alpha = (float)3e-3; // INTERFACE

// length of lines when reading file
#define LINELEN 80
#define FATAL(test, fmt, ...)                                                 \
    do {                                                                      \
        if (test) {                                                           \
            fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, \
                    __VA_ARGS__);                                             \
            fprintf(stderr, "exiting from fatal error\n");                    \
            exit(1);                                                          \
        }                                                                     \
    } while (0)

static inline void
read_sites_from_file(const char *path, point **arr_ptr, int32_t *nsites)
{
    FILE *file = fopen(path, "r");
    FATAL(file == NULL, "path not valid: '%s'\n", path);

    char line[LINELEN];
    (*arr_ptr) = malloc(2 * sizeof(point));
    int32_t allocated = 2;
    for ((*nsites) = 0; fgets(line, LINELEN, file) != NULL; (*nsites)++) {
        if ((*nsites) >= allocated) {
            allocated *= 2;
            *arr_ptr = realloc((*arr_ptr), sizeof(point) * (size_t)(allocated));
        }
        char *first = strtok(line, ", \t");
        char *second = strtok(NULL, ", \t");
        (*arr_ptr)[(*nsites)].x = strtof(first, NULL);
        (*arr_ptr)[(*nsites)].y = strtof(second, NULL);
    }
    fclose(file);
    (*arr_ptr) = realloc((*arr_ptr), (size_t)(*nsites) * sizeof(point));
}

static inline float
frac(float x)
{
    float useless_required_ptr;
    return modff(x, &useless_required_ptr);
}

static inline void
copy_edges(struct edgelist *edgelist, point *dest)
{
    for (int i = 0; i < edgelist->nedges; i++) {
        // multiply by 2 because: 1 edge = 2 points
        dest[i * 2].x = edgelist->edges[i]->origin.x;
        dest[i * 2].y = edgelist->edges[i]->origin.y;
        dest[i * 2 + 1].x = edgelist->edges[i]->twin->origin.x;
        dest[i * 2 + 1].y = edgelist->edges[i]->twin->origin.y;
    }
}

// -------------------------- ploting stuff ----------------------------
static inline void
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
verify_nsites(int nsites_found, int nsites)
{
    FATAL(nsites_found > nsites, "error: nsites found %d, nsites expected %d\n",
          nsites_found, nsites);
}

static inline void
first_step(point **sites,
           point **linesegs,
           const int nsites,
           const int points_per_trial,
           float *initial_perimeter,
           float *initial_objectivefunction,
           obj_func obj_function)
{
    point *sites_found;
    int32_t nsites_found;
    read_sites_from_file("input", &sites_found, &nsites_found);
    verify_nsites(nsites_found, nsites);

    struct edgelist edgelist_first_perimeter;
    init_edgelist(&edgelist_first_perimeter);
    fortunes(sites_found, nsites_found, &edgelist_first_perimeter);
    copy_edges(&edgelist_first_perimeter, &(*linesegs)[0 * points_per_trial]);
    memcpy(*sites, sites_found, (size_t)nsites * sizeof(point));
    free(sites_found);
    *initial_perimeter = calc_perimeter(&edgelist_first_perimeter);
    *initial_objectivefunction =
        obj_function(*sites, &edgelist_first_perimeter, nsites);
    free_edgelist(&edgelist_first_perimeter);
}

static inline void
calc_gradient_for_site(const int j,
                       const int nsites,
                       const point *const old_sites,
                       point *gradient,
                       const float jiggle,
                       const float prev_perimeter,
                       obj_func obj_function)
{
    point *local_sites = malloc((size_t)nsites * sizeof(point));
    memcpy(local_sites, old_sites, (size_t)nsites * sizeof(point));
    struct edgelist local_edgelist;
    // x
    local_sites[j].x = frac(local_sites[j].x + jiggle);
    init_edgelist(&local_edgelist);
    fortunes(local_sites, nsites, &local_edgelist);
    gradient[j].x =
        obj_function(local_sites, &local_edgelist, nsites) - prev_perimeter;
    gradient[j].x /= jiggle;
    local_sites[j].x = old_sites[j].x; // reset for y
    free_edgelist(&local_edgelist);    // reset for y
    // y
    local_sites[j].y = frac(local_sites[j].y + jiggle);
    init_edgelist(&local_edgelist);
    fortunes(local_sites, nsites, &local_edgelist);
    gradient[j].y =
        obj_function(local_sites, &local_edgelist, nsites) - prev_perimeter;
    gradient[j].y /= jiggle;

    free(local_sites);
    free_edgelist(&local_edgelist);
}

static inline void
calc_stats(struct edgelist *edgelist,
           point *sites,
           float *perimeter,
           float *objective_function,
           obj_func obj_function,
           int nsites)
{
    *perimeter = calc_perimeter(edgelist);
    *objective_function = obj_function(sites, edgelist, nsites);
}

void
gradient_descent(struct arrays arrs,
                 const float jiggle,
                 const int nsites,
                 const int pts_per_trial,
                 const int trials)
{
    grad_func method = calc_gradient_for_site; // INTERFACE
    obj_func obj_function = obj_perimeter_and_repel;     // INTERFACE

    point *linesegs = (point *)arrs.linesegs_to_be_cast;
    point *sites = (point *)arrs.sites_to_be_cast;
    float *perimeter = arrs.perimeter;
    float *obj_func_vals = arrs.objective_function;

    first_step(&sites, &linesegs, nsites, pts_per_trial, &perimeter[0],
               &obj_func_vals[0], obj_function);

    point *gradient = malloc((size_t)nsites * sizeof(point));
    for (int i = 1; i < trials; i++) { // start at 1: there is no prev perimeter
        memset(gradient, 0, (size_t)nsites * sizeof(point));
        float prev_perimeter = perimeter[i - 1];
        point *old_sites_ptr = &sites[(i - 1) * nsites];
        // PARALLEL
        for (int j = 0; j < nsites; j++)
            method(j, nsites, old_sites_ptr, gradient, jiggle, prev_perimeter,
                   obj_function);
        update_sites(old_sites_ptr, &sites[i * nsites], gradient, nsites);

        struct edgelist edgelist;
        init_edgelist(&edgelist);
        fortunes(&sites[i * nsites], nsites, &edgelist);
        copy_edges(&edgelist, &linesegs[i * pts_per_trial]);
        calc_stats(&edgelist, sites, &perimeter[i], &obj_func_vals[i],
                   obj_function, nsites);
        free_edgelist(&edgelist);
    }
    free(gradient);
}

void
simple_diagram(float *numpy_arr, int size, float *sites, int nsites_expected)
{
    struct edgelist e;
    init_edgelist(&e);
    point *sites_found;
    int32_t nsites_found;

    read_sites_from_file("input", &sites_found, &nsites_found);
    verify_nsites(nsites_found, nsites_expected);

    fortunes(sites_found, nsites_found, &e);
    FATAL(e.nedges > size, "error: size of numpy arr %d, num of edges %d\n",
          size, e.nedges);

    copy_edges(&e, (point *)numpy_arr);
    memcpy(sites, sites_found, (size_t)nsites_found * sizeof(point));
    free(sites_found);
    free_edgelist(&e);
}
// ------------------------- end of plot methods -------------------

#ifndef NMAIN
static inline void
print_sites(point *sites, int32_t length)
{
    for (int i = 0; i < length; i++)
        printf("(%f,%f),\t", (double)sites[i].x, (double)sites[i].y);
    printf("\n");
}

static void
default_graph(void)
{
    struct edgelist e;
    init_edgelist(&e);
    point sites[3] = {
        {(float)0.875, (float)0.169},
        {(float)0.852, (float)0.792},
        {(float)0.233, (float)0.434},
    };
    print_sites(sites, 3);
    fortunes(sites, 3, &e);
    print_edgelist(&e);
    free_edgelist(&e);
}

static void
graph_file(const char *path)
{
    struct edgelist e;
    init_edgelist(&e);
    int32_t nsites = 0;
    point *sites;
    read_sites_from_file(path, &sites, &nsites);
    print_sites(sites, nsites);
    fortunes(sites, nsites, &e);
    free(sites);
    print_edgelist(&e);
    free_edgelist(&e);
}

int
main(int argc, char **argv)
{
    if (argc == 1) {
        default_graph();
    } else {
        graph_file(argv[argc - 1]);
    }
    return 0;
}
#endif
