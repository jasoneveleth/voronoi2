#include <stdio.h>
#include "bintree.h"

bintree *
init_tree(void)
{
    bintree *tree = malloc(sizeof(bintree));
    tree->arr = malloc(sizeof(bnode *) * INIT_SIZE);
    tree->allocated = INIT_SIZE;
    tree->size = 0;
    return tree;
}

bnode *
binsert(bintree *tree, void *val, key key)
{
    bnode *node = malloc(sizeof(bnode));
    node->attr = val;
    node->key = key;

    tree->arr[1] = node;

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
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
    while (node && node->key != key) {
        if (node->key > key) {
            node = tree->arr[node->index * 2 + 1];
        } else if (node->key < key) {
            node = tree->arr[node->index * 2];
        }
    }
#pragma clang diagnostic pop
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
