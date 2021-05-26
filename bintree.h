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

typedef struct {
    int32_t index;
    char padding[4];
    void *attr;
} bnode;

typedef struct {
    bnode **arr;
    size_t allocated; // # of pointers
    int32_t size;
    char padding[4];
} bintree;

bintree *init_tree(void);
bnode *binsert(bintree *, void *, int32_t);
void *bremove(bintree *, bnode *);
bnode *bfindarc(bintree *, point);
int32_t bempty(bintree *);
bnode *bpredecessor(bintree *, bnode *);
bnode *bsuccessor(bintree *, bnode *);

bnode *bnextleaf(bintree *, bnode *);
bnode *bprevleaf(bintree *, bnode *);
bnode *bgetmin(bintree *, bnode *);
bnode *bgetmax(bintree *, bnode *);
int bisinternal(bintree *, bnode *);
// bnode *findMin(void);
// bnode *findMax(void);
point intersect_parabolas(float, point *);

#endif
