#include "heap.h"
#include <stdio.h>
#include <assert.h>

#ifdef DEBUG
static inline void
printall(heap *H)
{
    int32_t max = 5;
    int32_t end = max > H->last + 1 ? H->last + 1 : max;
    for (int i = 1; i < end; i++) {
        // printf("key: %.0f, val: %d, index: %d\n", (double)H->arr[i]->key,
        // *(int *)H->arr[i]->attr, H->arr[i]->index);
        printf("key: %.0f, val: %d, index: %d\n",
               (double)H->arr[i]->key,
               *(int *)H->arr[i]->attr,
               H->arr[i]->index);
    }
}
#endif

static inline hnode *
maxchild(heap *H, hnode *node)
{
    hnode *lchild = H->arr[node->index * 2];
    hnode *rchild =
        H->arr[node->index * 2 + 1];  // may be NULL -- but is handled properly
    if (node->index * 2 == H->last) { // no right child
        return lchild;
    } else {
        return lchild->key > rchild->key ? lchild : rchild;
    }
}

static inline void
downheap(heap *H, hnode *node)
{
    while (node->index * 2 <= H->last) { // has left child
        hnode *mchild = maxchild(H, node);
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
upheap(heap *H, hnode *node)
{
    while (
        node->index > 1
        && node->key
               > H->arr[node->index / 2]->key) { // has parent && key > parent
        hnode *parent = H->arr[node->index / 2];
        H->arr[parent->index] = node;
        H->arr[node->index] = parent;
        parent->index = node->index;
        node->index /= 2;
    }
}

int
hempty(heap *H)
{
    return H->last == 0;
}

void
free_heap(heap *H)
{
    free(H->arr);
    free(H);
}

heap *
init_heap(void)
{
    heap *H = malloc(sizeof(heap));
    H->arr = malloc(sizeof(hnode *) * INIT_SIZE);
    H->last = 0;
    H->allocated = INIT_SIZE;
    return H;
}

void *
hremove_max(heap *H)
{
    if (hempty(H)) fprintf(stderr, "Can't remove max, heap empty");
    hnode *max = H->arr[1];
    hnode *last = H->arr[H->last];
    void *val = max->attr;
#ifdef DEBUG
    puts("remove max ----");
    printf("(removing) key: %.0f, val: %d\n", (double)max->key, *(int *)val);
    printall(H);
    puts("/remove max");
#endif
    assert(max->index == 1);   // this check enforces current implementation
    H->arr[max->index] = last; // <--|  are implementation agnostic
    last->index = max->index;  // <--/

    H->last--;
    free(max);
    downheap(H, last);
    return val;
}

void *
hremove(heap *H, hnode *node)
{
    if (hempty(H)) fprintf(stderr, "Can't remove element, heap empty");
    hnode *last = H->arr[H->last];
    void *val = node->attr;
    H->arr[node->index] = last;
    last->index = node->index;
    H->last--;
    upheap(H, last);
    downheap(H, last);
    free(node);
#ifdef DEBUG
    printf("remove ----------- %d\n", *(int *)val);
    printall(H);
    puts("/remove");
#endif
    return val;
}

hnode *
hinsert(heap *H, void *attr, key key)
{
#ifdef DEBUG
    printf("insert ----------- %d\n", H->last);
    printall(H);
#endif
    H->last++;
    if (H->last
        >= H->allocated) { // (+1 used to) convert from index (last) to length
        // don't allow larger than 1GB, for the arr (doesn't count the nodes
        // themselves
        assert((H->last * ((long)sizeof(hnode *))) < 1024 * 1024 * 1024);
        H->allocated *= 2;
        H->arr = realloc(H->arr, sizeof(hnode) * (size_t)(H->allocated));
    }

    hnode *new = malloc(sizeof(hnode));
    H->arr[H->last] = new;
    new->attr = attr;
    new->index = H->last;
    new->key = key;

    upheap(H, new);
    downheap(H, new);
#ifdef DEBUG
    printf("-----------\n");
    printall(H);
    puts("/insert");
    // if (key - (float)348.0 < (float)1e-4) {
    //     puts("----------------------------\n\n-----------------------");
    // }
    // printall(H);
    // puts("++++++++++++++++++++++++++++\n\n+++++++++++++++++++++++");
#endif
    return new;
}
