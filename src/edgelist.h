#ifndef EDGELIST_H
#define EDGELIST_H

#include <stdint.h>
#include "geometry.h"

struct halfedge {
    point origin;
    struct halfedge *twin;
    struct halfedge *next;
    struct halfedge *prev;
    struct face *face;
};

struct face {
    struct halfedge *outercompnent;
    point site;
};

struct edgelist {
    struct face *face;
    struct halfedge **edges;
    int32_t allocated;
    int32_t nedges;
};

float calc_perimeter(struct edgelist *);
void copy_edges(struct edgelist *, point *);
void init_edgelist(struct edgelist *);
void calc_char_length(struct edgelist *, float *, float *);

#endif
