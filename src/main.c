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

static void
read_lines_from_file(FILE *file, point **sites_found, int *nsites_found)
{
    char line[LINELEN];
    (*sites_found) = malloc(2 * sizeof(point));
    int32_t allocated = 2;
    for ((*nsites_found) = 0; fgets(line, LINELEN, file) != NULL;
         (*nsites_found)++) {
        if ((*nsites_found) >= allocated) {
            allocated *= 2;
            (*sites_found) =
                realloc((*sites_found), sizeof(point) * (size_t)(allocated));
        }
        char *first = strtok(line, ", \t");
        char *second = strtok(NULL, ", \t");
        (*sites_found)[(*nsites_found)].x = strtof(first, NULL);
        (*sites_found)[(*nsites_found)].y = strtof(second, NULL);
    }
}

static inline void
read_sites_from_file(const char *path, point **sites, int32_t *nsites)
{
    point *sites_found;
    int32_t nsites_found;

    FILE *file = fopen(path, "r");
    FATAL(file == NULL, "path not valid: '%s'\n", path);
    read_lines_from_file(file, &sites_found, &nsites_found);
    fclose(file);

    sites_found = realloc(sites_found, (size_t)nsites_found * sizeof(point));
    verify_nsites(nsites_found, *nsites);

    // if the array 'sites' is too large, I change the int in nsites, (this
    // doesn't apply when (*sites == NULL), but it's still nice to know I guess
    if (*nsites != nsites_found) {
        fprintf(stderr, "changing nsites from: %d to %d\n", *nsites,
                nsites_found);
        *nsites = nsites_found;
    }
    // this is the case when we didn't know how big the file was poing to turn
    // out to be, so we don't destroy the array we've been reading into, rather
    // reassign
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
           float *char_max_length,
           float *char_min_length,
           int nsites)
{
    *perimeter = calc_perimeter(edgelist);
    *objective_function = obj_function(sites, edgelist, nsites);
    calc_char_length(edgelist, char_max_length, char_min_length);
}

static inline void
simple_descent(struct arrays numpy_arrs,
               const float jiggle,
               int nsites,
               const int pts_per_trial,
               const int trials)
{
    // unpack numpy arrays
    point *linesegs = (point *)numpy_arrs.linesegs_to_be_cast;
    point *sites = (point *)numpy_arrs.sites_to_be_cast;
    float *perimeter = numpy_arrs.perimeter;
    float *obj_func_vals = numpy_arrs.objective_function;
    float *char_max_length = numpy_arrs.char_max_length;
    float *char_min_length = numpy_arrs.char_min_length;

    read_sites_from_file("input", &sites, &nsites);

    point *gradient = malloc((size_t)nsites * sizeof(point));
    for (int i = 0; i < trials; i++) {
        if (i > 0) { // skip this the first time
            float prev_objective = obj_func_vals[i - 1];
            point *old_sites_ptr = &sites[(i - 1) * nsites];
            // PARALLEL
            for (int j = 0; j < nsites; j++)
                gradient_method(j, nsites, old_sites_ptr, gradient, jiggle,
                                prev_objective);
#ifdef REPEL
            float sumx = 0;
            float sumy = 0;
            for (int j = 0; j < nsites; j++) {
                sumx += gradient[j].x;
                sumy += gradient[j].y;
            }
            float avgx = sumx / (float)nsites;
            float avgy = sumy / (float)nsites;
            for (int j = 0; j < nsites; j++) {
                gradient[j].x -= avgx;
                gradient[j].y -= avgy;
            }
#endif
            static const float alpha = (float)3e-3; // MMM
            update_sites(old_sites_ptr, &sites[i * nsites], gradient, nsites,
                         alpha);
        }

        struct edgelist edgelist;
        init_edgelist(&edgelist);
        fortunes(&sites[i * nsites], nsites, &edgelist);
        copy_edges(&edgelist, &linesegs[i * pts_per_trial]);
        calc_stats(&edgelist, sites, &perimeter[i], &obj_func_vals[i],
                   &char_max_length[i], &char_min_length[0], nsites);
        free_edgelist(&edgelist);
    }
    free(gradient);
}

static inline void
barziilai_borwein(struct arrays numpy_arrs,
                  const float jiggle,
                  int nsites,
                  const int pts_per_trial,
                  const int trials)
{
    // unpack numpy arrays
    point *linesegs = (point *)numpy_arrs.linesegs_to_be_cast;
    point *sites = (point *)numpy_arrs.sites_to_be_cast;
    float *perimeter = numpy_arrs.perimeter;
    float *obj_func_vals = numpy_arrs.objective_function;
    float *char_max_length = numpy_arrs.char_max_length;
    float *char_min_length = numpy_arrs.char_min_length;

    read_sites_from_file("input", &sites, &nsites);

    point *g_k = malloc((size_t)nsites * sizeof(point));
    point *g_k1 = malloc((size_t)nsites * sizeof(point));
    for (int i = 0; i < trials; i++) {
        if (i > 0) { // skip this the first time
            float prev_objective = obj_func_vals[i - 1];
            point *old_sites_ptr = &sites[(i - 1) * nsites];
            // PARALLEL
            for (int j = 0; j < nsites; j++)
                gradient_method(j, nsites, old_sites_ptr, g_k, jiggle,
                                prev_objective);
#ifdef REPEL
            float sumx = 0;
            float sumy = 0;
            for (int j = 0; j < nsites; j++) {
                sumx += g_k[j].x;
                sumy += g_k[j].y;
            }
            float avgx = sumx / (float)nsites;
            float avgy = sumy / (float)nsites;
            for (int j = 0; j < nsites; j++) {
                g_k[j].x -= avgx;
                g_k[j].y -= avgy;
            }
#endif
            float alpha;
            if (i == 1) {
                alpha = 3e-3f; // MMM
            } else {
                point *x_k1 = &sites[(i - 2) * nsites];
                point *x_k = &sites[(i - 1) * nsites];
                alpha = bb_formula(x_k1, x_k, g_k1, g_k, nsites);
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
        calc_stats(&edgelist, sites, &perimeter[i], &obj_func_vals[i],
                   &char_max_length[i], &char_min_length[0], nsites);
        free_edgelist(&edgelist);
    }
    free(g_k);
    free(g_k1);
}

void
gradient_descent(struct arrays numpy_arrs,
                 const float jiggle,
                 int nsites,
                 const int pts_per_trial,
                 const int trials)
{
    barziilai_borwein(numpy_arrs, jiggle, nsites, pts_per_trial, trials);
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
