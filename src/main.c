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
binary_write(const char *const file, void *const buf, const size_t length)
{
    // HARDCODE char = 1 byte (for nice readablility)
    char *path = calloc(1, strlen(file) + strlen(options.output_dir) + 1);
    strcpy(path, options.output_dir);
    strcat(path, file);

    FILE *fp = fopen(path, "wb");
    FATAL(!fp, "path: %s doesn't exist\n", path);
    fwrite(buf, length, 1, fp);
    fclose(fp);
}

static void
calc_earth_mover(size_t nbins, float *earthmover, float *edgehist, int i)
{
    if (i > 0) {
        float *old_hist = &edgehist[(i - 1) * (int)nbins];
        float *new_hist = &edgehist[i * (int)nbins];
        float total = 0;
        float ith = 0;
        for (int j = 1; j < (int)nbins; j++) {
            ith = old_hist[j] + ith - new_hist[j];
            total += fabsf(ith);
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
                 float *edgehist,
                 float *earthmover,
                 size_t ntrials,
                 size_t nsites)
{
    const float maxdist = 1.4143f; // HARDCODE
    const size_t nbins = (size_t)(maxdist * (float)nsites);
    size_t nedges = 3 * nsites - 6;
    for (size_t i = 0; i < ntrials; i++) {
        max[i] = 0.0f;
        min[i] = 1.0f / 0.0f;
        size_t edges_count = 0;
        for (size_t j = 0; j < nedges; j++) {
            float x1 = linesegs[i * 2 * nedges + j * 2].x;
            float y1 = linesegs[i * 2 * nedges + j * 2].y;
            float x2 = linesegs[i * 2 * nedges + j * 2 + 1].x;
            float y2 = linesegs[i * 2 * nedges + j * 2 + 1].y;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
            if (x1 == 0 && x2 == 0 && y1 == 0 && y2 == 0) {
#pragma GCC diagnostic pop
                edges_count++;
            } else {
                float dist =
                    sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
                edgehist[i * nbins + (size_t)(dist * (float)nsites)] += 1.0f;
                if (dist > max[i]) max[i] = dist;
                if (dist < min[i]) min[i] = dist;
            }
        }
        for (size_t k = 0; k < nbins; k++)
            edgehist[i * nbins + k] *= 1.0f / (float)(nedges - edges_count);
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
    binary_write("linesegs", better, filelen * 2);
    free(better);
}

static void
calc_arrays(void)
{
    struct arrays arrs;
    size_t nsites = (size_t)get_nsites("input");
    size_t nbins = (size_t)(1.4143f * (float)nsites);
    // HARDCODE
    size_t size_of_edgehist = options.ntrials * nbins;

    size_t nbytes;
    char *path = calloc(1, strlen(options.output_dir) + strlen("edges") + 1);
    strcpy(path, options.output_dir);
    strcat(path, "edges");
    arrs.linesegs = from_file(path, &nbytes);
    edges2linesegs(arrs.linesegs, nbytes, nsites);

    arrs.edgehist = calloc(size_of_edgehist, sizeof(float));
    arrs.earthmover = malloc(options.ntrials * sizeof(float));
    arrs.char_max_length = malloc(options.ntrials * sizeof(float));
    arrs.char_min_length = malloc(options.ntrials * sizeof(float));

    calc_edge_length(arrs.linesegs, arrs.char_max_length, arrs.char_min_length,
                     arrs.edgehist, arrs.earthmover, options.ntrials, nsites);
    free(arrs.linesegs);

    nbytes = sizeof(arrs.char_max_length[0]) * options.ntrials;
    binary_write("char_max_length", arrs.char_max_length, nbytes);

    nbytes = sizeof(arrs.char_min_length[0]) * options.ntrials;
    binary_write("char_min_length", arrs.char_min_length, nbytes);

    nbytes = sizeof(arrs.edgehist[0]) * nbins * options.ntrials;
    binary_write("edgehist", arrs.edgehist, nbytes);

    nbytes = sizeof(arrs.earthmover[0]) * options.ntrials;
    binary_write("earthmover", arrs.earthmover, nbytes);

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
    binary_write("sites", sites, nsites * sizeof(point));
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
    binary_write("linesegs", better, size_of_edgelist * 2);
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
    file2sites(options.filepath, &arrs.sites, &nsites);
    size_t pts_per_trial = (3 * nsites - 6) * (2);
    size_t num_of_linsegs = options.ntrials * pts_per_trial;

    arrs.linesegs = calloc(num_of_linsegs, sizeof(point));
    arrs.objective_function = malloc(options.ntrials * sizeof(float));
    arrs.perimeter = malloc(options.ntrials * sizeof(float));
    arrs.alpha = malloc(options.ntrials * sizeof(float));

    arrs.alpha[0] = 0; // there is no step first.
    gradient_descent(arrs, (int)nsites, (int)pts_per_trial);
    size_t nbytes;
    nbytes = sizeof(arrs.linesegs[0]) * options.ntrials * (3 * nsites - 6) * 2;
    binary_write("edges", arrs.linesegs, nbytes);

    nbytes = sizeof(arrs.sites[0]) * (size_t)nsites * options.ntrials;
    binary_write("sites", arrs.sites, nbytes);

    nbytes = sizeof(arrs.perimeter[0]) * (size_t)options.ntrials;
    binary_write("perimeter", arrs.perimeter, nbytes);

    nbytes = sizeof(arrs.objective_function[0]) * options.ntrials;
    binary_write("objective_function", arrs.objective_function, nbytes);

    nbytes = sizeof(arrs.alpha[0]) * options.ntrials;
    binary_write("alpha", arrs.alpha, nbytes);

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
    options.output_dir = DEFAULT_OUTPUT_DIR;
    options.obj = PERIMETER | REPULSION;
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
            if (strchr("bcjs", optarg[0]) == NULL) {
                usage(argv[0], optopt);
                return EXIT_FAILURE;
            }
            switch (optarg[0]) {
            case 'c':
                options.descent = CONSTANT_ALPHA;
                break;
            case 'b':
                options.descent = BARZILAI;
                break;
            case 'j':
                options.descent = CONJUGATE;
                break;
            case 's':
                options.descent = STEEPEST;
                break;
            }
            break;

        case 'n':
            options.ntrials = (size_t)atoi(optarg);
            break;

        case 'f':
            {
                size_t len = strlen(optarg);
                FATAL(optarg[len - 1] != '/', "%s\n",
                      "outputdir needs to end with '/'");
                options.output_dir = optarg;
            }
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
