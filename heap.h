#ifndef HEAP_H
#define HEAP_H
#include <stdint.h>
#include <stdlib.h>
#define INIT_SIZE 1024

#ifdef FLOAT
typedef float key;
#else
typedef int32_t key;
#endif

typedef struct {
    key key;
    int32_t index;
    void *attr;
} hnode;

typedef struct {
    hnode **arr;
    int32_t last;
    int32_t allocated;
} heap;

void free_heap(heap *);
heap *init_heap(void);
void *hremove_max(heap *);
void *hremove(heap *, hnode *);
int hempty(heap *);
hnode *hinsert(heap *, void *, key);

// TODO NEED TO FREE ATTR

#endif
