#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "main.h"

static inline void
verify_nsites(int nsites_found, int nsites)
{
    FATAL(nsites_found > nsites, "error: nsites found %d, nsites expected %d\n",
          nsites_found, nsites);
}

static inline void
read_sites_from_file(const char *path, point **sites, int32_t *nsites)
{
    point *sites_found;
    int32_t nsites_found;
    FILE *file = fopen(path, "r");
    FATAL(file == NULL, "path not valid: '%s'\n", path);

    char line[LINELEN];
    sites_found = malloc(2 * sizeof(point));
    int32_t allocated = 2;
    for (nsites_found = 0; fgets(line, LINELEN, file) != NULL; nsites_found++) {
        if (nsites_found >= allocated) {
            allocated *= 2;
            sites_found =
                realloc(sites_found, sizeof(point) * (size_t)(allocated));
        }
        char *first = strtok(line, ", \t");
        char *second = strtok(NULL, ", \t");
        sites_found[nsites_found].x = strtof(first, NULL);
        sites_found[nsites_found].y = strtof(second, NULL);
    }
    fclose(file);
    sites_found = realloc(sites_found, (size_t)nsites_found * sizeof(point));
    verify_nsites(nsites_found, *nsites);
    if (*nsites != nsites_found) {
        fprintf(stderr, "changing nsites from: %d to %d\n", *nsites,
                nsites_found);
        *nsites = nsites_found;
    }
    if (*sites == NULL) {
        *sites = sites_found;
    } else {
        memcpy(*sites, sites_found, (size_t)nsites_found * sizeof(point));
        free(sites_found);
    }
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

static inline void
first_step(point **sites,
           point **linesegs,
           int nsites,
           const int points_per_trial,
           float *initial_perimeter,
           float *initial_objectivefunction,
           obj_func obj_function)
{
    read_sites_from_file("input", sites, &nsites);

    struct edgelist edgelist_first_perimeter;
    init_edgelist(&edgelist_first_perimeter);
    fortunes(*sites, nsites, &edgelist_first_perimeter);
    copy_edges(&edgelist_first_perimeter, &(*linesegs)[0 * points_per_trial]);
    calc_stats(&edgelist_first_perimeter, *sites, initial_perimeter,
               initial_objectivefunction, obj_function, nsites);
    free_edgelist(&edgelist_first_perimeter);
}

static inline void
calc_gradient_for_site(const int j,
                       const int nsites,
                       const point *const old_sites,
                       point *gradient,
                       const float jiggle,
                       const float prev_objective,
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
        obj_function(local_sites, &local_edgelist, nsites) - prev_objective;
    gradient[j].x /= jiggle;
    local_sites[j].x = old_sites[j].x; // reset for y
    free_edgelist(&local_edgelist);    // reset for y
    // y
    local_sites[j].y = frac(local_sites[j].y + jiggle);
    init_edgelist(&local_edgelist);
    fortunes(local_sites, nsites, &local_edgelist);
    gradient[j].y =
        obj_function(local_sites, &local_edgelist, nsites) - prev_objective;
    gradient[j].y /= jiggle;

    free(local_sites);
    free_edgelist(&local_edgelist);
}

void
gradient_descent(struct arrays arrs,
                 const float jiggle,
                 const int nsites,
                 const int pts_per_trial,
                 const int trials)
{
    grad_func method = calc_gradient_for_site; // INTERFACE
    obj_func obj_function = obj_perimeter;     // INTERFACE

    point *linesegs = (point *)arrs.linesegs_to_be_cast;
    point *sites = (point *)arrs.sites_to_be_cast;
    float *perimeter = arrs.perimeter;
    float *obj_func_vals = arrs.objective_function;

    first_step(&sites, &linesegs, nsites, pts_per_trial, &perimeter[0],
               &obj_func_vals[0], obj_function);

    point *gradient = malloc((size_t)nsites * sizeof(point));
    for (int i = 1; i < trials; i++) { // start at 1: there is no prev perimeter
        memset(gradient, 0, (size_t)nsites * sizeof(point));
        float prev_objective = obj_func_vals[i - 1];
        point *old_sites_ptr = &sites[(i - 1) * nsites];
        // PARALLEL
        for (int j = 0; j < nsites; j++)
            method(j, nsites, old_sites_ptr, gradient, jiggle, prev_objective,
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
simple_diagram(float *numpy_arr,
               int size,
               float *sites_numpy_arr,
               int nsites_expected)
{
    point *sites = (point *)sites_numpy_arr;
    read_sites_from_file("input", &sites, &nsites_expected);
    struct edgelist e;
    init_edgelist(&e);

    fortunes(sites, nsites_expected, &e);
    FATAL(e.nedges > size, "error: size of numpy arr %d, num of edges %d\n",
          size, e.nedges);

    copy_edges(&e, (point *)numpy_arr);
    free_edgelist(&e);
}

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
    int32_t nsites = 1000000; // limmit to 1_000_000
    point *sites = NULL;
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
    } else if (argv[1][1] == 'p') {
        graph_file(argv[argc - 1]);
    } else {
        graph_file(argv[argc - 1]);
    }
    return 0;
}
#endif

/* vim: set ft=c.makemaps: */
