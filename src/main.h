#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fortunes.h"
#include "cython_stuff.h"
#include "gradient.h"

typedef float (*obj_func)(point *sites, struct edgelist *, int);
typedef void (*grad_func)(const int,
                          const int,
                          const point *const,
                          point *,
                          const float,
                          const float,
                          obj_func);

// static const float alpha = (float)3e-3; // INTERFACE
static const float alpha = (float)1e-5; // INTERFACE

// length of lines when reading file
#define LINELEN 80
#define FATAL(test, fmt, ...)                                                 \
    do {                                                                      \
        if (test) {                                                           \
            fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, \
                    __VA_ARGS__);                                             \
            fprintf(stderr, "exiting from fatal error\n");                    \
            exit(1);                                                          \
        }                                                                     \
    } while (0)

#endif
