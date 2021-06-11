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

void
gradient_descent(struct arrays numpy_arrs,
                 const float jiggle,
                 int nsites,
                 const int pts_per_trial,
                 const int trials)
{
    read_sites_from_file("input", (point **)&numpy_arrs.sites_to_be_cast,
                         &nsites);
    simple_descent(numpy_arrs, jiggle, nsites, pts_per_trial, trials);
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
