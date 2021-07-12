#ifndef MAC
#define _GNU_SOURCE
#include <fenv.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
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

static long
get_nsites(const char *const path)
{
    FILE *infile = fopen(path, "r");
    FATAL(!infile, "file: %s doesn't exist\n", path);

    long nsites = 0;
    int c;
    int lastchar = '\0';
    while ((c = fgetc(infile)) != EOF) {
        lastchar = c;
        if (c == '\n') nsites++;
    }
    // if last character isn't a new line add it anyway
    if (lastchar != '\n') nsites++;

    fclose(infile);
    return nsites;
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
calc_earth_mover(size_t nbins, int *earthmover, int *edgehist, int i)
{
    if (i > 0) {
        int *old_hist = &edgehist[(i - 1) * (int)nbins];
        int *new_hist = &edgehist[i * (int)nbins];
        int total = 0;
        int ith = 0;
        for (int j = 1; j < (int)nbins; j++) {
            ith = old_hist[j] + ith - new_hist[j];
            total += abs(ith);
        }
        earthmover[i] = total;
    } else {
        earthmover[i] = 0;
    }
}

static void
calc_edge_length(struct point *linesegs,
                 float *max,
                 float *min,
                 int *edgehist,
                 int *earthmover,
                 size_t ntrials,
                 size_t nsites)
{
    const float maxdist = 1.4143f; // HARDCODE
    const size_t nbins = (size_t)(maxdist * (float)nsites);
    size_t nedges = 3 * nsites - 6;
    for (size_t i = 0; i < ntrials; i++) {
        max[i] = 0.0f;
        min[i] = 1.0f / 0.0f;
        for (size_t j = 0; j < nedges; j++) {
            float x1 = linesegs[i * 2 * nedges + j * 2].x;
            float y1 = linesegs[i * 2 * nedges + j * 2].y;
            float x2 = linesegs[i * 2 * nedges + j * 2 + 1].x;
            float y2 = linesegs[i * 2 * nedges + j * 2 + 1].y;
            float dist = sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
            edgehist[i * nbins + (size_t)(dist * (float)nsites)]++;
            if (dist > max[i]) max[i] = dist;
            if (dist < min[i]) min[i] = dist;
        }
        calc_earth_mover(nbins, earthmover, edgehist, (int)i);
    }
}

static void *
from_file(const char *const path, size_t *filelen)
{
    // https://stackoverflow.com/questions/22059189/read-a-file-as-byte-array
    FILE *fileptr;
    void *buffer;

    fileptr = fopen(path, "rb"); // Open the file in binary mode
    FATAL(!fileptr, "file: %s doesn't exist\n", path);
    fseek(fileptr, 0, SEEK_END); // Jump to the end of the file
    *filelen =
        (size_t)ftell(fileptr); // Get the current byte offset in the file
    rewind(fileptr);            // Jump back to the beginning of the file

    buffer = malloc(*filelen);           // Enough memory for the file
    fread(buffer, *filelen, 1, fileptr); // Read in the entire file
    fclose(fileptr);                     // Close the file
    return buffer;
}

static void
edges2linesegs(point *buffer, size_t filelen, size_t nsites)
{
    point *better = malloc(filelen * 2);
    size_t shape = 3 * nsites - 6;
    for (size_t i = 0; i < options.ntrials; i++) {
        for (size_t j = 0; j < shape; j++) {
            better[i * 4 * shape + j * 4] = buffer[i * 2 * shape + j * 2];
            better[i * 4 * shape + j * 4 + 1] =
                buffer[i * 2 * shape + j * 2 + 1];
            better[i * 4 * shape + j * 4 + 2] =
                buffer[i * 2 * shape + j * 2 + 1];
            better[i * 4 * shape + j * 4 + 3] = buffer[i * 2 * shape + j * 2];
        }
    }
    binary_write("output/linesegs", better, filelen * 2);
    free(better);
}

static void
calc_arrays(void)
{
    struct arrays arrs;
    size_t nsites = (size_t)get_nsites("input");
    // HARDCODE
    size_t size_of_edgehist =
        options.ntrials * (size_t)(1.4143f * (float)nsites);

    size_t nbytes;
    arrs.linesegs = from_file("output/edges", &nbytes);
    edges2linesegs(arrs.linesegs, nbytes, nsites);

    arrs.edgehist = calloc(1, size_of_edgehist * sizeof(float));
    arrs.earthmover = malloc(options.ntrials * sizeof(float));
    arrs.char_max_length = malloc(options.ntrials * sizeof(float));
    arrs.char_min_length = malloc(options.ntrials * sizeof(float));

    calc_edge_length(arrs.linesegs, arrs.char_max_length, arrs.char_min_length,
                     arrs.edgehist, arrs.earthmover, options.ntrials, nsites);

    nbytes = sizeof(arrs.char_max_length[0]) * options.ntrials;
    binary_write("output/char_max_length", arrs.char_max_length, nbytes);

    nbytes = sizeof(arrs.char_min_length[0]) * options.ntrials;
    binary_write("output/char_min_length", arrs.char_min_length, nbytes);

    nbytes = sizeof(arrs.edgehist[0]) * (size_t)((float)nsites * 1.4143f)
             * options.ntrials;
    binary_write("output/edgehist", arrs.edgehist, nbytes);

    nbytes = sizeof(arrs.earthmover[0]) * options.ntrials;
    binary_write("output/earthmover", arrs.earthmover, nbytes);

    free(arrs.edgehist);
    free(arrs.char_max_length);
    free(arrs.char_min_length);
    free(arrs.earthmover);
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

    // misleading bc really its 2 pts per halfedge, but we only care about 1/2
    // halfedges
    size_t size_of_edgelist = sizeof(point) * (size_t)e.nedges;
    point *edges = malloc(size_of_edgelist);
    copy_edges(&e, edges);
    // binary_write("output/linesegs", edges, size_of_edgelist);

    // preserve tests
    point *better = calloc(1, size_of_edgelist * 2);
    for (int j = 0; j < e.nedges / 2; j++) {
        better[j * 4] = edges[j * 2];
        better[j * 4 + 1] = edges[j * 2 + 1];
        better[j * 4 + 2] = edges[j * 2 + 1];
        better[j * 4 + 3] = edges[j * 2];
    }
    binary_write("output/linesegs", better, size_of_edgelist * 2);
    free(better);
    // preserve tests

    free(edges);
    free_edgelist(&e);
}

static void
big_func()
{
    struct arrays arrs;
    size_t nsites;
    file2sites(options.filepath, (point **)&arrs.sites, &nsites);
    size_t pts_per_trial = (3 * nsites - 6) * (2);
    size_t size_of_linsegs = options.ntrials * pts_per_trial;

    arrs.linesegs = calloc(1, size_of_linsegs * sizeof(point));
    arrs.objective_function = malloc(options.ntrials * sizeof(float));
    arrs.perimeter = malloc(options.ntrials * sizeof(float));
    arrs.alpha = malloc(options.ntrials * sizeof(float));

    arrs.alpha[0] = 0; // there is no step first.
    gradient_descent(arrs, options.jiggle, (int)nsites, (int)pts_per_trial);
    size_t nbytes;
    nbytes = sizeof(arrs.linesegs[0]) * options.ntrials * (3 * nsites - 6) * 2;
    binary_write("output/edges", arrs.linesegs, nbytes);

    nbytes = sizeof(arrs.sites[0]) * (size_t)nsites * options.ntrials;
    binary_write("output/sites", arrs.sites, nbytes);

    nbytes = sizeof(arrs.perimeter[0]) * (size_t)options.ntrials;
    binary_write("output/perimeter", arrs.perimeter, nbytes);

    nbytes = sizeof(arrs.objective_function[0]) * options.ntrials;
    binary_write("output/objective_function", arrs.objective_function, nbytes);

    nbytes = sizeof(arrs.alpha[0]) * options.ntrials;
    binary_write("output/alpha", arrs.alpha, nbytes);

    free(arrs.linesegs);
    free(arrs.objective_function);
    free(arrs.perimeter);
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
        case 's':
            options.silent = true;
            break;
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

    if (options.ntrials == 0) {
        graph_file(options.filepath);
    } else {
        big_func();
        calc_arrays();
    }
    return EXIT_SUCCESS;
}

/* vim: set ft=c.makemaps: */
