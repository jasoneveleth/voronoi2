#ifndef HEAP_H
#define HEAP_H
#include <stdint.h>
#define INIT_SIZE 1024

// This is a general purpose heap, so it works with floats and integers as
// keys. In this case we are using floats. Here we define the 'key' type to
// whichever we are using.
typedef float key;

// Nodes store a void * which is the data that they hold, the key which is used
// for up and down heaping and the index so each node knows where it is.
struct hnode {
    key key;
    int32_t index;
    void *attr;
};

// The 'last' field is the index of the last element. Because heaps are left
// complete binary trees, we can use the inate structure of an array's indices
// to organize our tree. To get to a parent, divide the index by 2, to get to
// the child, multiply the index by 2 and add 0 to go left, and add 1 to go
// right. Draw out a complete binary tree using just numbers starting from 1 and
// numbering the vertices from top to bottom from left to right to see how this
// works.
struct heap {
    struct hnode **arr;
    int32_t last;
    int32_t allocated;
};

// Standard heap functions. They don't need explaination
void free_heap(struct heap *);
struct heap *init_heap(void);
void *hremove_max(struct heap *);
void *hremove(struct heap *, struct hnode *);
int hempty(struct heap *);
struct hnode *hinsert(struct heap *, void *, key);
void print_heap(struct heap *);

#endif
