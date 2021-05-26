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
    struct halfedge *edges[2];
    char padding[4];
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

typedef union event event;
union event {
    struct site_event {
        char kind;
        char padding[3];
        point site;
    } site_event;
    struct circle_event {
        char kind;
        char padding[7];
        point lowest_point;
        struct arc *arc;
    } circle_event;
};

void fortunes(point *, int32_t, struct edgelist *);

#endif
