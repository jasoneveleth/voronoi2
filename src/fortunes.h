#ifndef FORTUNES_H
#define FORTUNES_H

#include "heap.h"
#include "bintree.h"
#include "edgelist.h"

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
void init_edgelist(struct edgelist *);
void free_edgelist(struct edgelist *);
void print_edgelist(struct edgelist *);

#endif
