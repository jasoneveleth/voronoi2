#ifndef MAC
#define _GNU_SOURCE
#include <fenv.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "main.h"

// *** NEVER MODIFY THIS EXCEPT IN init_options() ***
struct options options;

void
init_options(const char *filepath,
             unsigned char obj,
             unsigned char descent,
             unsigned char gradient,
             unsigned char boundary,
             float alpha,
             float repel_coeff,
             int ntrials,
             float jiggle)
{
    options.filepath = filepath;
    options.obj = obj;
    options.descent = descent;
    options.gradient = gradient;
    options.boundary = boundary;
    options.alpha = alpha;
    options.repel_coeff = repel_coeff;
    options.ntrials = ntrials;
    options.jiggle = jiggle;
}

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
gradient_descent(struct arrays arrs, int nsites, const int pts_per_trial)
{
    read_sites_from_file(options.filepath, (point **)&arrs.sites_to_be_cast,
                         &nsites);
    if (options.descent == CONSTANT_ALPHA) {
        simple_descent(arrs, options.jiggle, nsites, pts_per_trial);
    } else if (options.descent == BARZIILAI) {
        barziilai_borwein(arrs, options.jiggle, nsites, pts_per_trial);
    } else {
        FATAL(1, "%s\n", "unreachable code");
    }
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
#ifndef MAC
    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif
    if (argc == 1) {
        default_graph();
    } else if (argv[1][1] == 't') {
        size_t nsites = strtoul(argv[2], NULL, 10);
        size_t ntrials = strtoul(argv[3], NULL, 10);
        init_options("input", PERIMETER, CONSTANT_ALPHA, FINITE_DIFFERENCE,
                     TORUS, 3e-3f, 1e-4f, (int)ntrials, 1e-4f);
        size_t linesegs_per_trial = 2 * (3 * nsites - 6);
        size_t pts_per_lineseg = 2;
        size_t floats_per_pt = 2;
        struct arrays arrs;
        arrs.linesegs_to_be_cast =
            malloc(ntrials * linesegs_per_trial * pts_per_lineseg
                   * floats_per_pt * sizeof(float));
        arrs.sites_to_be_cast =
            malloc(ntrials * nsites * floats_per_pt * sizeof(float));
        arrs.perimeter = malloc(ntrials * sizeof(float));
        arrs.objective_function = malloc(ntrials * sizeof(float));
        arrs.char_max_length = malloc(ntrials * sizeof(float));
        arrs.char_min_length = malloc(ntrials * sizeof(float));
        gradient_descent(arrs, (int)nsites,
                         (int)(linesegs_per_trial * pts_per_lineseg));

        FILE *gnuplotPipe = popen("gnuplot -persistent", "w");
        fputs("set terminal gif animate delay 20\n", gnuplotPipe);
        fputs("set output 'newest.gif'\n", gnuplotPipe);
        fputs(
            "set style line 1 linecolor rgb '#0060ad' linetype 1 linewidth 1 "
            "pointsize 0\n",
            gnuplotPipe);

        fputs("set xrange [0:1]\n", gnuplotPipe);
        fputs("set yrange [0:1]\n", gnuplotPipe);
        //  4 becuase we have each line segment on there twice, and its 2 points
        //  in a lineseg
        size_t inner_loop_size = 4 * (3 * nsites - 6);
        for (size_t i = 0; i < ntrials; i++) {
            fputs("plot '-' with linespoints linestyle 1\n", gnuplotPipe);
            for (size_t j = 0; j < inner_loop_size; j += 2) {
                point start =
                    ((point *)(arrs.linesegs_to_be_cast))[i * inner_loop_size
                                                          + j];
                point end =
                    ((point *)(arrs.linesegs_to_be_cast))[i * inner_loop_size
                                                          + j + 1];
                fprintf(gnuplotPipe, "%f %f\n%f %f\n\n", (double)start.x,
                        (double)start.y, (double)end.x, (double)end.y);
            }
            fputs("e\n", gnuplotPipe);
        }
        fprintf(gnuplotPipe, "exit");
        pclose(gnuplotPipe);
        // for (int i = 0; i < ntrials; i++) { printf("%f\n",
        // arrs.perimeter[i]); } printf("\n");
        // size_t len = ntrials * linesegs_per_trial * pts_per_lineseg
        //              * floats_per_pt;
        // for (size_t i = 0; i < len; i += 4) {
        //     printf("(%f, %f) (%f, %f)\n", arrs.linesegs_to_be_cast[i],
        //            arrs.linesegs_to_be_cast[i + 1],
        //            arrs.linesegs_to_be_cast[i + 2],
        //            arrs.linesegs_to_be_cast[i + 3]);
        // }
        // for (int i = 0; i < ntrials; i++) {
        //     for (int j = 0; j < nsites; j++) {
        //         printf("(%f, %f)\n",
        //                ((point *)(arrs.sites_to_be_cast))[i * nsites + j].x,
        //                ((point *)(arrs.sites_to_be_cast))[i * nsites + j].y);
        //     }
        // }
        // printf("\n");
        // for (int i = 0; i < ntrials; i++) {
        //     for (int j = 0; j < nsites; j++) { printf(""); }
        // }
    } else {
        graph_file(argv[argc - 1]);
    }
    return 0;
}
#endif

/* vim: set ft=c.makemaps: */
