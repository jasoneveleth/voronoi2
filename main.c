#include <stdio.h>
#include <string.h>
#include "bintree.h"
#include "fortunes.h"
#include "heap.h"

#include "cython_stuff.h"
#include "cython_stuff.c"

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
    if (argc == 1) {
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
    } else {
        struct edgelist e;
        init_edgelist(&e);
        int32_t nsites = 0;
        point *sites;
        read_sites_from_file(argv[argc - 1], &sites, &nsites);
        print_sites(sites, nsites);
        fortunes(sites, nsites, &e);
        free(sites);
        print_edgelist(&e);
        free_edgelist(&e);
    }
    return 0;
}
