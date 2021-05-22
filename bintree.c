#include <stdio.h>
#include "bintree.h"
#include <assert.h>
#include <string.h>

void *
realloc_zero(void *old, size_t oldsize, size_t newsize)
{
    void *new = realloc(old, newsize);
    if (newsize > oldsize && new) {
        size_t diff = newsize - oldsize;
        void *start = ((char *)new) + oldsize;
        memset(start, 0, diff);
    }
    return new;
}

bintree *
init_tree(void)
{
    bintree *tree = malloc(sizeof(bintree));
    tree->arr = calloc(INIT_SIZE, sizeof(bnode *));
    tree->allocated = INIT_SIZE;
    tree->size = 0;
    return tree;
}

bnode *
binsert(bintree *tree, void *val, key key)
{
    if (tree->size * 2 >= tree->allocated) { // allocated is a length
        // no larger than 1GB for the arr (doesn't count the nodes memory)
        assert((tree->size * ((long)sizeof(bnode *))) < 1024 * 1024 * 1024);
        tree->allocated *= 2;
        tree->arr =
            realloc(tree->arr, sizeof(bnode *) * (size_t)(tree->allocated));
    }
    bnode *node = malloc(sizeof(bnode));
    node->attr = val;
    node->key = key;
    node->height = 0;
    node->index = -1;

    bnode **dest = &tree->arr[1];
    while (*dest != NULL && (*dest)->key != key) {
        if ((*dest)->key > key) {
            dest = &tree->arr[(*dest)->index * 2 + 1];
        } else if ((*dest)->key < key) {
            dest = &tree->arr[(*dest)->index * 2];
        } else {
            break;
        }
    }
    *dest = node; // will be assigned to arr[1] if assigning root
    tree->size++;

    return node;
}

void *
bremove(bintree *tree, bnode *node)
{
    free(tree->arr[node->index]);
    return node->attr;
}

bnode *
bfind(bintree *tree, key key)
{
    bnode *node = tree->arr[1];
    // we need to let float be equal for keys
    while (node != NULL && node->key != key) {
        if (node->key > key) {
            node = tree->arr[node->index * 2 + 1];
        } else if (node->key < key) {
            node = tree->arr[node->index * 2];
        }
    }
    return node;
}

int32_t
bempty(bintree *tree)
{
    return tree->size == 0;
}

// bnode *bpredecessor(bnode *) {

// }

// bnode *bsuccessor(bnode *) {

// }

// root()
// isleaf()
// isroot()
// addroot()

// addright()
// left()
// right()
// hasleft()
// hasright()
