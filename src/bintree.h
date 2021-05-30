#ifndef BINTREE_H
#define BINTREE_H

#define INIT_SIZE 1024
#include <stdint.h>
#include <stdlib.h>

typedef struct point point;
struct point {
    float x;
    float y;
};

struct bnode {
    struct bnode *left;
    struct bnode *right;
    struct bnode *parent;
    union {
        struct bp *bp;
        struct arc *arc;
    };
};

struct bnode *baddleft(struct bnode *, void *);
struct bnode *baddright(struct bnode *, void *);
void *bremove(struct bnode *);
struct bnode *bfindarc(struct bnode *, point);
struct bnode *bpredecessor(struct bnode *);
struct bnode *bsuccessor(struct bnode *);

struct bnode *bnextleaf(struct bnode *);
struct bnode *bprevleaf(struct bnode *);
struct bnode *bgetmin(struct bnode *);
struct bnode *bgetmax(struct bnode *);
void free_tree(struct bnode *);
int bisinternal(struct bnode *);
point intersect_parabolas(float, point *);
float fsqrt(float);

#endif
