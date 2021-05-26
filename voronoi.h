#ifndef VORONOI_H
#define VORONOI_H

#include "heap.h"
#include "bintree.h"

// These two structs depend on each other, so I have to declare them both before
// I can define them
typedef struct halfedge halfedge;
typedef struct face face;

struct halfedge {
    point origin;
    halfedge *twin;
    halfedge *next;
    halfedge *prev;
    face *face;
};

struct face {
    halfedge *outercompnent;
    point site;
};

typedef struct edgelist edgelist;
struct edgelist {
    face *face;
    halfedge *edges[2];
    char padding[4];
    int32_t nedges;
};

typedef struct bp bp;
struct bp {
    point sites[2];
    halfedge *edge;
};

typedef struct arc arc;
struct arc {
    hnode *circle_event;
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
        arc *arc;
    } circle_event;
};

void fortunes(point *, int32_t, edgelist *);

#endif
