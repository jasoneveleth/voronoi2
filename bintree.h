#ifndef BINTREE_H
#define BINTREE_H
#define INIT_SIZE 1024
#include <stdint.h>
#include <stdlib.h>

#ifdef FLOAT
typedef float key;
#else
typedef int32_t key;
#endif

typedef struct {
    int32_t index;
    key key;
    void *attr;
} bnode;

typedef struct {
    bnode **arr;
    int32_t size;
    int32_t allocated;
} bintree;

bintree *init_tree(void);
bnode *binsert(bintree *, void *, key);
void *bremove(bintree *, bnode *);
bnode *bfind(bintree *, key);
int32_t bempty(bintree *);
bnode *bpredecessor(bintree *, bnode *);
bnode *bsuccessor(bintree *, bnode *);

// bnode *nextleaf(bnode *);
// bnode *prevleaf(bnode *);
// bnode *findMin(void);
// bnode *findMax(void);

#endif
