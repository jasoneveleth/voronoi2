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

static void
print_sites(point *sites, int32_t length)
{
    for (int i = 0; i < length; i++)
        printf("(%f,%f),\t", (double)sites[i].x, (double)sites[i].y);
    printf("\n");
}

int
main(int argc, char **argv)
{
    struct edgelist e;
    init_edgelist(&e);
    if (argc == 1) {
        point sites[3] = {
            {(float)0.875, (float)0.169},
            {(float)0.852, (float)0.792},
            {(float)0.233, (float)0.434},
        };
        print_sites(sites, 3);
        fortunes(sites, 3, &e);
    } else {
        int32_t nsites = 0;
        point *sites;
        read_sites_from_file(argv[argc - 1], &sites, &nsites);
        print_sites(sites, nsites);
        fortunes(sites, nsites, &e);
        free(sites);
    }
    print_edgelist(&e);
    free_edgelist(&e);
    return 0;
}
