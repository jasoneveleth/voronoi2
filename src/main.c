#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bintree.h"
#include "fortunes.h"
#include "heap.h"
#include "cython_stuff.h"

// length of lines when reading file
#define LINELEN 80

static inline void
read_sites_from_file(const char *path, point **arr_ptr, int32_t *length)
{
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "path not valid: %s\n", path);
        exit(1);
    }
    char line[LINELEN];
    point *arr = malloc(2 * sizeof(point));
    int32_t allocated = 2;
    int nsites;
    for (nsites = 0; fgets(line, LINELEN, file) != NULL; nsites++) {
        if (nsites >= allocated) {
            allocated *= 2;
            arr = realloc(arr, sizeof(point) * (size_t)(allocated));
        }
        char *first = strtok(line, ", \t");
        char *second = strtok(NULL, ", \t");
        arr[nsites].x = strtof(first, NULL);
        arr[nsites].y = strtof(second, NULL);
    }
    fclose(file);
    arr = realloc(arr, (size_t)nsites * sizeof(point));
    *length = nsites;
    *arr_ptr = arr;
}

static void
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
void
monte_carlo(float *edges,
            float *sites,
            float *perimeter,
            float jiggle,
            int nsites,
            int trials)
{
}

static void
update_sites(point *src, point *dest, point *grad, int nsites)
{
}

void
gradient_descent(float *edges_to_be_cast,
                 float *sites_to_be_cast,
                 float *perimeter,
                 float jiggle,
                 int nsites,
                 int trials)
{
    point *edges = (point *)edges_to_be_cast;
    point *sites = (point *)sites_to_be_cast;
    size_t edges_trial_size = (3 * nsites - 6) * 2; // (in terms of points)
    struct edgelist e;
    init_edgelist(&e);

    point *sites_found;
    int32_t nsites_found;
    read_sites_from_file("input", &sites_found, &nsites_found);
    if (nsites_found > nsites) {
        fprintf(stderr,
                "error: nsites found %d, nsites expected %d\n",
                nsites_found,
                nsites);
        fprintf(stderr, "exiting from fatal error\n");
        exit(1);
    }

    fortunes(sites_found, nsites_found, &e);
    copy_edges(&e, &edges[0 * edges_trial_size]);
    memcpy(sites, sites_found, nsites * sizeof(point));
    free(sites_found);
    perimeter[0] = calc_perimeter(&e);
    free_edgelist(&e);

    point *gradient = malloc(nsites * sizeof(point));
    for (int i = 1; i < trials; i++) {
        // start at 1, becuase there is no prev perimeter
        memset(gradient, 0, sizeof(point) * nsites);
        float prev_perimeter = perimeter[i - 1];
        for (int j = 0; j < nsites; j++) {
            point *local_sites = malloc(nsites * sizeof(point));
            memcpy(
                local_sites, &sites[(i - 1) * nsites], nsites * sizeof(point));
            struct edgelist edgelist;
            // x
            local_sites[j].x += jiggle;
            init_edgelist(&edgelist);
            fortunes(local_sites, nsites, &edgelist);
            gradient[j].x = calc_perimeter(&edgelist) - prev_perimeter;

            local_sites[j].x = sites[(i - 1) * nsites + j].x;
            free_edgelist(&edgelist);
            // y
            local_sites[j].y += jiggle;
            init_edgelist(&edgelist);
            fortunes(local_sites, nsites, &edgelist);
            gradient[j].y = calc_perimeter(&edgelist) - prev_perimeter;

            free(local_sites);
            free_edgelist(&edgelist);
        }
        update_sites(
            &sites[(i - 1) * nsites], &sites[i * nsites], gradient, nsites);
        struct edgelist edgelist;
        fortunes(&sites[i * nsites], nsites, &edgelist);
        copy_edges(&edgelist, &edges[i * edges_trial_size]);
        perimeter[i] = calc_perimeter(&edgelist);
        free_edgelist(&edgelist);
    }
}

void
simple_diagram(float *numpy_arr, int size, float *sites, int nsites_expected)
{
    struct edgelist e;
    init_edgelist(&e);
    point *sites_found;
    int32_t nsites_found;

    read_sites_from_file("input", &sites_found, &nsites_found);
    if (nsites_found > nsites_expected) {
        fprintf(stderr,
                "nsites found %d, nsites expected %d\n",
                nsites_found,
                nsites_expected);
        fprintf(stderr, "exiting from fatal error\n");
        exit(1);
    }

    fortunes(sites_found, nsites_found, &e);
    if (e.nedges > size) {
        fprintf(stderr,
                "error: size of numpy arr %d, num of edges %d\n",
                size,
                e.nedges);
        fprintf(stderr, "exiting from fatal error\n");
        exit(1);
    }

    for (int i = 0; i < e.nedges; i++) {
        // multiply by 4 because: 1 edge = 2 points = 4 floats
        numpy_arr[i * 4] = e.edges[i]->origin.x;
        numpy_arr[i * 4 + 1] = e.edges[i]->origin.y;
        numpy_arr[i * 4 + 2] = e.edges[i]->twin->origin.x;
        numpy_arr[i * 4 + 3] = e.edges[i]->twin->origin.y;
    }

    // TODO make this memcpy
    for (int i = 0; i < nsites_found; i++) {
        // multiply by 2 because: 1 site = 1 points = 2 floats
        sites[2 * i] = sites_found[i].x;
        sites[2 * i + 1] = sites_found[i].y;
    }

    free(sites_found);
    free_edgelist(&e);
}
// ------------------------- end of plot methods -------------------

#ifndef NMAIN
static void
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
