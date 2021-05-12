#ifndef HEAP_H
#define HEAP_H
#include <stdlib.h>
#define INIT_SIZE 1024

typedef float key_t;
// typedef int32_t key_t;

typedef struct {
    key_t key;
    int32_t index;
    void *attr;
} hnode_t;

typedef struct {
    hnode_t *arr;
    int32_t last;
    int32_t allocated;
} heap_t;

void free_heap(heap_t *);
heap_t *init_heap(void);
void *hremove_max(heap_t *);
void hremove(heap_t *, hnode_t *);
int hempty(heap_t *);
// void hdecrease_key(heap_t *, hnode_t *, int32_t);
hnode_t *hinsert(heap_t *, void *, key_t);

// TODO NEED TO FREE ATTR

// I often will leave the pointers as their old names when manipulating
// (upheaping and downheaping) in their new positions.
#endif
