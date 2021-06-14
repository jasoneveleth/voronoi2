#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "heap.h"

void
print_heap(struct heap *H)
{
    int32_t max = 5;
    int32_t end = max > H->last + 1 ? H->last + 1 : max;
    for (int i = 1; i < end; i++) {
        fprintf(stderr, "key: %f, val: %d, index: %d\n", (double)H->arr[i]->key,
                *(int *)H->arr[i]->attr, H->arr[i]->index);
    }
}

static inline struct hnode *
maxchild(struct heap *H, struct hnode *node)
{
    struct hnode *lchild = H->arr[node->index * 2];
    struct hnode *rchild =
        H->arr[node->index * 2 + 1];  // may be NULL -- but is handled properly
    if (node->index * 2 == H->last) { // no right child
        return lchild;
    } else {
        return lchild->key > rchild->key ? lchild : rchild;
    }
}

static inline void
downheap(struct heap *H, struct hnode *node)
{
    while (node->index * 2 <= H->last) { // has left child
        struct hnode *mchild = maxchild(H, node);
        if (node->key < mchild->key) {
            H->arr[mchild->index] = node;
            H->arr[node->index] = mchild;
            node->index = mchild->index;
            mchild->index /= 2;
        } else {
            break;
        }
    }
}

static inline void
upheap(struct heap *H, struct hnode *node)
{
    while (
        node->index > 1
        && node->key
               > H->arr[node->index / 2]->key) { // has parent && key > parent
        struct hnode *parent = H->arr[node->index / 2];
        H->arr[parent->index] = node;
        H->arr[node->index] = parent;
        parent->index = node->index;
        node->index /= 2;
    }
}

int
hempty(struct heap *H)
{
    return H->last == 0;
}

void
free_heap(struct heap *H)
{
    assert(hempty(H));
    free(H->arr);
    free(H);
}

struct heap *
init_heap(void)
{
    struct heap *H = malloc(sizeof(struct heap));
    H->arr = malloc(sizeof(struct hnode *) * INIT_SIZE);
    H->last = 0;
    H->allocated = INIT_SIZE;
    return H;
}

void *
hremove_max(struct heap *H)
{
    if (hempty(H)) fprintf(stderr, "Can't remove max, heap empty");
    struct hnode *max = H->arr[1];
    struct hnode *last = H->arr[H->last];
    void *val = max->attr;
    assert(max->index == 1);   // this check enforces current implementation
    H->arr[max->index] = last; // <--|  are implementation agnostic
    last->index = max->index;  // <--/

    H->last--;
    downheap(H, last);
    free(max);
    return val;
}

void *
hremove(struct heap *H, struct hnode *node)
{
    if (hempty(H)) fprintf(stderr, "Can't remove element, heap empty");
    struct hnode *last = H->arr[H->last];
    void *val = node->attr;
    H->arr[node->index] = last;
    last->index = node->index;
    H->last--;
    upheap(H, last);
    downheap(H, last);
    free(node);
    return val;
}

struct hnode *
hinsert(struct heap *H, void *attr, key key)
{
    H->last++;                     // is an index
    if (H->last >= H->allocated) { // allocated is a length
        // no larger than 1GB for the arr (doesn't count the nodes memory)
        assert((H->last * ((long)sizeof(struct hnode *))) < 1024 * 1024 * 1024);
        H->allocated *= 2;
        H->arr =
            realloc(H->arr, sizeof(struct hnode *) * (size_t)(H->allocated));
    }

    struct hnode *new = malloc(sizeof(struct hnode));
    H->arr[H->last] = new;
    new->attr = attr;
    new->index = H->last;
    new->key = key;

    upheap(H, new);
    downheap(H, new);
    return new;
}
