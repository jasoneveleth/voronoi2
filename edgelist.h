#ifndef HEAP_H
#define HEAP_H

// This is just to store an x and y coordinate
typedef struct point point;
struct point {
    float x;
    float y;
};

// These two structs depend on each other, so I have to declare them both before
// I can define them. They are the real output of the voronoi diagram.
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
};

// I don't know what this will have yet.
typedef struct edgelist edgelist;
struct edgelist {
    ;
};

#endif
