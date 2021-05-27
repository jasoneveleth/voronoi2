#ifndef VORONOI_H
#define VORONOI_H

#include "heap.h"
#include "bintree.h"

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

struct bp {
    point sites[2];
    struct halfedge *edge;
};

struct arc {
    struct hnode *circle_event;
    point site;
};

struct event {
    char kind;
    char padding[7];
    union {
        point site;
        struct {
            point circle_bottom;
            struct bnode *leaf;
        };
    };
};

void fortunes(point *, int32_t, struct edgelist *);

#endif
