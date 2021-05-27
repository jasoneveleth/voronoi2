#ifndef BINTREE_H
#define BINTREE_H

#define INIT_SIZE 1024
#include <stdint.h>
#include <stdlib.h>

#ifdef FLOAT
typedef float bkey;
#else
typedef int32_t bkey;
#endif

typedef struct point point;
struct point {
    float x;
    float y;
};

struct bnode {
    int32_t index;
    char padding[4];
    union {
        struct bp *bp;
        struct arc *arc;
    };
};

struct bintree {
    struct bnode **arr;
    size_t allocated; // # of pointers
    int32_t size;
    char padding[4];
};

struct bintree *init_tree(void);
struct bnode *binsert(struct bintree *, void *, int32_t);
void *bremove(struct bintree *, struct bnode *);
struct bnode *bfindarc(struct bintree *, point);
int32_t bempty(struct bintree *);
struct bnode *bpredecessor(struct bintree *, struct bnode *);
struct bnode *bsuccessor(struct bintree *, struct bnode *);

struct bnode *bnextleaf(struct bintree *, struct bnode *);
struct bnode *bprevleaf(struct bintree *, struct bnode *);
struct bnode *bgetmin(struct bintree *, struct bnode *);
struct bnode *bgetmax(struct bintree *, struct bnode *);
int bisinternal(struct bintree *, struct bnode *);
point intersect_parabolas(float, point *);
float fsqrt(float);

#endif
