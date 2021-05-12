#include "heap.h"
#include <stdio.h>
#include <assert.h>

#ifdef DEBUG
static inline void 
printall(heap_t *H) 
{
    int32_t max = 5;
    int32_t end = max > H->last + 1 ? H->last + 1 : max;
    for (int i = 1; i < end; i++) {
        // printf("key: %.0f, val: %d, index: %d\n", (double)H->arr[i]->key, *(int *)H->arr[i]->attr, H->arr[i]->index);
        printf("key: %.0f, val: %d\n", (double)H->arr[i]->key, *(int *)H->arr[i]->attr);
    }
}
#endif

static inline hnode_t *
maxchild(heap_t *H, hnode_t *node)
{
    hnode_t *lchild = H->arr[node->index * 2];
    hnode_t *rchild = H->arr[node->index * 2 + 1]; // may be NULL -- but is handled properly
    if (node->index*2 == H->last) { // no right child
        return lchild;
    } else  {
        return lchild->key > rchild->key ? lchild : rchild;
    }
}

static inline void 
downheap(heap_t *H, hnode_t *node) 
{
    while (node->index*2 <= H->last) { // has left child 
        hnode_t *mchild = maxchild(H, node);
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
upheap(heap_t *H, hnode_t *node) 
{
    while (node->index > 1 && node->key > H->arr[node->index/2]->key) { // has parent && key > parent
        hnode_t *parent = H->arr[node->index/2];
        H->arr[parent->index] = node;
        H->arr[node->index] = parent;
        parent->index = node->index;
        node->index /= 2;
    }
}

int 
hempty(heap_t *H) 
{
    return H->last == 0;
}

void 
free_heap(heap_t *H) 
{
    free(H->arr);
    free(H);
}

heap_t *
init_heap(void) 
{
    heap_t *H = malloc(sizeof(heap_t));
    H->arr = malloc(sizeof(hnode_t *)*INIT_SIZE);
    H->last = 0;
    H->allocated = INIT_SIZE;
    return H;
}

void *
hremove_max(heap_t *H) 
{
    if (hempty(H)) fprintf(stderr, "Can't remove max, heap empty");
    hnode_t *max = H->arr[1];
    hnode_t *last = H->arr[H->last];
    void *val = max->attr;
#ifdef DEBUG
    puts("remove max ----");
    printf("(removing) key: %.0f, val: %d\n", (double)max->key, *(int *)val);
    printall(H);
    puts("/remove max");
#endif
    assert(max->index == 1); // this check enforces current implementation
    H->arr[max->index] = last; // <--|  are implementation agnostic
    last->index = max->index; // <--/

    H->last--;
    free(max);
    downheap(H, last);
    return val;
}

void *
hremove(heap_t *H, hnode_t *node) 
{
    if (hempty(H)) fprintf(stderr, "Can't remove element, heap empty");
    hnode_t *last = H->arr[H->last];
    void *val = node->attr;
    H->arr[node->index] = last;
    H->last--;
    free(node);
    upheap(H, last);
    downheap(H, last);
    return val;
}

hnode_t *
hinsert(heap_t *H, void *attr, key_t key) 
{
#ifdef DEBUG
    // printf("insert ----------- %d\n", H->last);
    // printall(H);
    // puts("/insert");
#endif
    H->last++;
    if (H->last >= H->allocated) { // (+1 used to) convert from index (last) to length
        // don't allow larger than 2GB, caps at 6.7108864e7
        assert((H->last * (4 + 4 + (long)sizeof(void *))) > 2*1024*1024*1024);
        H->allocated *= 2;
        H->arr = realloc(H->arr, sizeof(hnode_t) * (size_t)(H->allocated));
    }

    hnode_t *new = malloc(sizeof(hnode_t));
    H->arr[H->last] = new;
    new->attr = attr;
    new->index = H->last;
    new->key = key;

    upheap(H, new);
    downheap(H, new);
#ifdef DEBUG
    // if (key - (float)348.0 < (float)1e-4) {
    //     puts("----------------------------\n\n-----------------------");
    // }
    // printall(H);
    // puts("++++++++++++++++++++++++++++\n\n+++++++++++++++++++++++");
#endif
    return new;
}

