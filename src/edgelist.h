#ifndef EDGELIST_H
#define EDGELIST_H

#include <stdint.h>
#include "geometry.h"

// These two structs depend on each other, so I have to declare them both before
// I can define them

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

#endif
