#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

#define FATAL(test, fmt, ...)                                                 \
    do {                                                                      \
        if (test) {                                                           \
            fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, \
                    __VA_ARGS__);                                             \
            fprintf(stderr, "exiting from fatal error\n");                    \
            exit(1);                                                          \
        }                                                                     \
    } while (0)

#ifndef NTHREADS
#define NTHREADS 1
#endif

enum __attribute__((packed)) objective_function
{
    PERIMETER = 1,
    REPULSION = 2,
    // powers of 2, becuase it's a set
};

// *** IMPORTANT number of types ****** //
#define NDESCENTTYPES 3
enum __attribute__((packed)) descent_method
{
    CONSTANT_ALPHA,
    BARZILAI,
    CONJUGATE
};

enum __attribute__((packed)) gradient_method
{
    FINITE_DIFFERENCE,
};

enum __attribute__((packed)) boundary_condition
{
    BOUNCE,
    TORUS,
};

// all need to be nonzero when used
struct options {
    const char *filepath;
    enum objective_function obj;
    enum descent_method descent;
    enum gradient_method gradient;
    enum boundary_condition boundary;
    float alpha;
    float repel_coeff;
    float jiggle;
    unsigned long ntrials;
    bool silent;
    char padding[7];
};

#endif
