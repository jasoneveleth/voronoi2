#ifndef MAC
#define _GNU_SOURCE
#include <fenv.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "main.h"

// *** NEVER MODIFY THIS EXCEPT IN main() ***
struct options options;

static void
read_lines_from_file(FILE *file, point **sites, size_t *nsites)
{
    char line[LINELEN];
    (*sites) = malloc(2 * sizeof(point));
    size_t allocated = 2;
    for ((*nsites) = 0; fgets(line, LINELEN, file) != NULL; (*nsites)++) {
        if ((*nsites) >= allocated) {
            allocated *= 2;
            (*sites) = realloc((*sites), sizeof(point) * allocated);
        }
        char *first = strtok(line, "(), \t");
        char *second = strtok(NULL, "(), \t");
        (*sites)[(*nsites)].x = strtof(first, NULL);
        (*sites)[(*nsites)].y = strtof(second, NULL);
    }
}

static inline void
file2sites(const char *path, point **sites, size_t *nsites)
{
    FILE *file = fopen(path, "r");
    FATAL(file == NULL, "path not valid: '%s'\n", path);
    read_lines_from_file(file, sites, nsites);
    fclose(file);

    if (options.ntrials == 0) {
        (*sites) = realloc((*sites), (*nsites) * sizeof(point));
    } else {
        size_t nbytes = options.ntrials * (*nsites) * sizeof(point);
        (*sites) = realloc((*sites), nbytes);
    }
}

static void
binary_write(const char *const path, void *const buf, const size_t length)
{
    FILE *file = fopen(path, "wb");
    FATAL(!file, "path: %s doesn't exist\n", path);
    fwrite(buf, length, 1, file);
    fclose(file);
}

static void
graph_file(const char *path)
{
    struct edgelist e;
    init_edgelist(&e);
    point *sites;
    size_t nsites;

    file2sites(path, &sites, &nsites);
    binary_write("output/sites", sites, nsites * sizeof(point));
    fortunes(sites, (int)nsites, &e);
    free(sites);

    size_t size_of_edgelist = sizeof(point) * 2 * (size_t)e.nedges;
    point *edges = malloc(size_of_edgelist);
    copy_edges(&e, edges);
    binary_write("output/linesegs", edges, size_of_edgelist);

    free(edges);
    free_edgelist(&e);
}

static void
output_to_file(struct arrays arrs, size_t nsites)
{
    size_t nbytes;
    nbytes =
        sizeof(arrs.linesegs[0]) * options.ntrials * 2 * (3 * nsites - 6) * 2;
    binary_write("output/linesegs", arrs.linesegs, nbytes);

    nbytes = sizeof(arrs.sites[0]) * (size_t)nsites * options.ntrials;
    binary_write("output/sites", arrs.sites, nbytes);

    nbytes = sizeof(arrs.perimeter[0]) * (size_t)options.ntrials;
    binary_write("output/perimeter", arrs.perimeter, nbytes);

    nbytes = sizeof(arrs.objective_function[0]) * options.ntrials;
    binary_write("output/objective_function", arrs.objective_function, nbytes);

    nbytes = sizeof(arrs.char_max_length[0]) * options.ntrials;
    binary_write("output/char_max_length", arrs.char_max_length, nbytes);

    nbytes = sizeof(arrs.char_min_length[0]) * options.ntrials;
    binary_write("output/char_min_length", arrs.char_min_length, nbytes);

    nbytes = sizeof(arrs.edgehist[0]) * (size_t)((float)nsites * 1.4143f)
             * options.ntrials;
    binary_write("output/edgehist", arrs.edgehist, nbytes);

    nbytes = sizeof(arrs.earthmover[0]) * options.ntrials;
    binary_write("output/earthmover", arrs.earthmover, nbytes);

    nbytes = sizeof(arrs.alpha[0]) * options.ntrials;
    binary_write("output/alpha", arrs.alpha, nbytes);
}

static void
big_func()
{
    if (options.ntrials == 0) {
        graph_file(options.filepath);
        return;
    }

    struct arrays arrs;
    size_t nsites;
    file2sites(options.filepath, (point **)&arrs.sites, &nsites);
    size_t pts_per_trial = (2 * (3 * nsites - 6)) * (2);
    size_t size_of_linsegs = options.ntrials * (2 * (3 * nsites - 6)) * (2);
    // HARDCODE
    size_t size_of_edgehist = options.ntrials * (nsites * 14143) / 10000;

    arrs.linesegs = calloc(1, size_of_linsegs * sizeof(point));
    arrs.edgehist = calloc(1, size_of_edgehist * sizeof(float));
    arrs.objective_function = malloc(options.ntrials * sizeof(float));
    arrs.char_max_length = malloc(options.ntrials * sizeof(float));
    arrs.char_min_length = malloc(options.ntrials * sizeof(float));
    arrs.perimeter = malloc(options.ntrials * sizeof(float));
    arrs.earthmover = malloc(options.ntrials * sizeof(float));
    arrs.alpha = malloc(options.ntrials * sizeof(float));

    arrs.alpha[0] = 0; // there is no step first.
    gradient_descent(arrs, options.jiggle, (int)nsites, (int)pts_per_trial);
    output_to_file(arrs, nsites);
    free(arrs.linesegs);
    free(arrs.edgehist);
    free(arrs.objective_function);
    free(arrs.char_max_length);
    free(arrs.char_min_length);
    free(arrs.perimeter);
    free(arrs.earthmover);
    free(arrs.alpha);
    free(arrs.sites);
}

static void
usage(char *progname, int wrong)
{
    if (wrong != 0) fprintf(stderr, "unknown option: %c\n", wrong);
    fprintf(stderr, USAGE_FMT, progname ? progname : DEFAULT_PROGNAME);
}

int
main(int argc, char **argv)
{
#ifndef MAC
    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif
    int opt;
    opterr = 0;
    options.filepath = DEFAULT_INPUTFILE;
    options.obj = PERIMETER;
    options.obj |= REPULSION;
    options.descent = CONSTANT_ALPHA;
    options.gradient = FINITE_DIFFERENCE;
    options.boundary = BOUNCE;
    options.alpha = 3e-3f;
    options.repel_coeff = 1e-4f;
    options.ntrials = 50;
    options.jiggle = 1e-4f;

    while ((opt = getopt(argc, argv, OPTSTR)) != EOF) {
        switch (opt) {
        case 'i':
            options.filepath = optarg;
            break;

        case 'b':
            if (optarg[0] == 't') {
                options.boundary = TORUS;
            } else if (optarg[0] == 'b') {
                options.boundary = BOUNCE;
            } else {
                usage(argv[0], optopt);
                return EXIT_FAILURE;
            }
            break;

        case 'o':
            for (int i = 0; optarg[i] != 0; i++) {
                switch (optarg[i]) {
                case 'p':
                    options.obj = PERIMETER;
                    break;
                case 'r':
                    options.obj |= REPULSION;
                    break;
                default:
                    usage(argv[0], optopt);
                    return EXIT_FAILURE;
                }
            }
            break;

        case 'd':
            if (optarg[0] == 'c') {
                options.descent = CONSTANT_ALPHA;
            } else if (optarg[0] == 'b') {
                options.descent = BARZILAI;
            } else {
                usage(argv[0], optopt);
                return EXIT_FAILURE;
            }
            break;

        case 'n':
            options.ntrials = (size_t)atoi(optarg);
            break;

        case 'h':
        case '?':
        default:
            usage(argv[0], optopt);
            return EXIT_FAILURE;
        }
    }

    big_func();
    return EXIT_SUCCESS;
}

/* vim: set ft=c.makemaps: */
