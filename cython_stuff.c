#include <stdio.h>
#include <string.h>
#include "bintree.h"
#include "fortunes.h"
#include "heap.h"

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

void simple_diagram(float *numpy_arr, int size)
{
    struct edgelist e;
    init_edgelist(&e);
    point *sites;
    int32_t nsites;
    read_sites_from_file("tests/sites/simple.in", &sites, &nsites);
    fortunes(sites, nsites, &e);

    for (int i = 0, n = 0; i < e.nedges; i++) {
        numpy_arr[n] = e.edges[i]->origin.x;
        numpy_arr[n + 1] = e.edges[i]->origin.y;
        numpy_arr[n + 2] = e.edges[i]->twin->origin.x;
        numpy_arr[n + 3] = e.edges[i]->twin->origin.y;
        // increase by 4 because: 1 edge = 2 points = 4 floats
        n += 4;
    }

    free(sites);
    free_edgelist(&e);
}
