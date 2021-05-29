#include <stdio.h>
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

    for (int i = 0; i < nsites_found; i++) {
        // multiply by 2 because: 1 site = 1 points = 2 floats
        sites[2 * i] = sites_found[i].x;
        sites[2 * i + 1] = sites_found[i].y;
    }

    free(sites_found);
    free_edgelist(&e);
}

static void
print_sites(point *sites, int32_t length)
{
    for (int i = 0; i < length; i++)
        printf("(%f,%f),\t", (double)sites[i].x, (double)sites[i].y);
    printf("\n");
}

#ifndef NMAIN
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
