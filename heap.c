#include "heap.h"
#include <stdio.h>
#include <assert.h>

static inline key_t max(key_t key1, key_t key2) {
    return key1 > key2 ? key1 : key2;
}

// static inline void printall(heap_t *H) {
//     // for (int i = 0; i < H->last+1; i++) {
//     for (int i = 0; i < 10; i++) {
//         printf("key: %f, val: %d\n", (double)H->arr[i].key, *(int *)H->arr[i].attr);
//     }
// }

static inline hnode_t *downheap(heap_t *H, hnode_t *node) {
    while (node->index*2 <= H->last && node->key < max(H->arr[node->index*2].key, H->arr[node->index*2 + 1].key)) {
        int32_t switching_index;
        if (node->index*2 == H->last || H->arr[node->index*2].key > H->arr[node->index*2 + 1].key) { // no right child || left > right
            switching_index = node->index*2;
        } else { // right child bigger
            switching_index = node->index*2 + 1;
        }
        hnode_t temp = H->arr[switching_index];
        H->arr[switching_index].attr = node->attr;
        H->arr[switching_index].key = node->key;
        node->attr = temp.attr;
        node->key = temp.key;
        node = &(H->arr[switching_index]);
    }
    return node;
}

static inline hnode_t *upheap(heap_t *H, hnode_t *node) {
    while (node->index > 0 && node->key > H->arr[node->index/2].key) {
        int32_t parent = node->index/2;
        hnode_t temp = H->arr[parent]; // TODO see if it copied struct
        H->arr[parent].attr = node->attr;
        H->arr[parent].key = node->key;
        node->attr = temp.attr;
        node->key = temp.key;
        node = &(H->arr[parent]);
    }
    return node;
}

void free_heap(heap_t *H) {
    free(H->arr);
    free(H);
}

heap_t *init_heap(void) {
    heap_t *H = malloc(sizeof(heap_t));
    H->arr = malloc(sizeof(hnode_t)*INIT_SIZE);
    H->last = -1;
    H->allocated = INIT_SIZE;
    return H;
}

void *hremove_max(heap_t *H) {
    if (H->last == -1) fprintf(stderr, "Can't remove max, heap empty");
    hnode_t *max = &(H->arr[0]);
    hnode_t last = H->arr[H->last];
    void *temp = max->attr;
    max->attr = last.attr;
    max->key = last.key;

    H->last--;
    downheap(H, max);
    return temp;
}

void hremove(heap_t *H, hnode_t *node) {
    hnode_t last = H->arr[H->last];
    node->attr = last.attr;
    node->key = last.key;
    H->last--;
    node = upheap(H, node);
    node = downheap(H, node); // TODO is this the right pointer?
}

// void hdecrease_key(heap_t *, hnode_t *) {

// }

hnode_t *hinsert(heap_t *H, void *attr, key_t key) {
    H->last++;
    if (H->last > H->allocated) {
        // don't allow larger than 2GB, caps at 6.7108864e7
        assert((H->last * (4 + 4 + (long)sizeof(void *))) > 2*1024*1024*1024);
        H->allocated *= 2;
        H->arr = realloc(H->arr, sizeof(hnode_t) * (size_t)(H->allocated));
    }
    hnode_t *new = &(H->arr[H->last]);
    new->attr = attr;
    new->index = H->last;
    new->key = key;
    new = upheap(H, new);
    new = downheap(H, new);
    return new;
}
