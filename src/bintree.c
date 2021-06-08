#include <stdio.h>
#include "bintree.h"
#include "fortunes.h"
#include <assert.h>
#include <string.h>
#include <math.h>

int
bisinternal(struct bnode *node)
{
    return node != NULL && (node->left != NULL || node->right != NULL);
}

struct bnode *
baddleft(struct bnode *root, void *val)
{
    struct bnode *node = malloc(sizeof(struct bnode));
    node->parent = root;
    node->left = NULL;
    node->right = NULL;
    node->bp = val; // can be either ->bp or ->arc
    root->left = node;
    return node;
}

struct bnode *
baddright(struct bnode *root, void *val)
{
    struct bnode *node = malloc(sizeof(struct bnode));
    node->parent = root;
    node->left = NULL;
    node->right = NULL;
    node->bp = val; // can be either ->bp or ->arc
    root->right = node;
    return node;
}

void *
bremove(struct bnode *node)
{
    void *attr = node->arc; // can be either -> arc or ->bp
    if (node->parent) {
        if (node->parent->left == node)
            node->parent->left = NULL;
        else
            node->parent->right = NULL;
    }
    free(node);
    return attr;
}

struct bnode *
bfindarc(struct bnode *root, point site)
{
    struct bnode *node = root;
    float sweepline = site.y;
    while (bisinternal(node)) {
        struct bp *breakpoint = node->bp;
        point intersection = intersect_parabolas(sweepline, breakpoint->sites);
        if (site.x > intersection.x) {
            node = node->right;
        } else {
            node = node->left;
        }
    }
    return node;
}

struct bnode *
bpredecessor(struct bnode *node)
{
    if (node->left == NULL) {
        struct bnode *child = node;
        node = node->parent;
        while (node != NULL) {
            if (node->right == child) return node;
            child = node;
            node = node->parent;
        }
        assert(node == NULL);
        return node;
    } else {
        return bgetmax(node->left);
    }
}

struct bnode *
bgetmin(struct bnode *node)
{
    while (node->left != NULL) { node = node->left; }
    return node;
}

struct bnode *
bgetmax(struct bnode *node)
{
    while (node->right != NULL) { node = node->right; }
    return node;
}

struct bnode *
bsuccessor(struct bnode *node)
{
    if (node->right == NULL) {
        struct bnode *child = node;
        node = node->parent;
        while (node != NULL) {
            if (node->left == child) return node;
            child = node;
            node = node->parent;
        }
        assert(node == NULL);
        return node;
    } else {
        return bgetmin(node->right);
    }
}

static inline struct bnode *
blowestleaf(struct bnode *node)
{
    while (1) {
        if (node->left != NULL) {
            node = node->left;
        } else if (node->right != NULL) {
            node = node->right;
        } else {
            return node;
        }
    }
}

static inline struct bnode *
bhighestleaf(struct bnode *node)
{
    while (1) {
        if (node->right != NULL) {
            node = node->right;
        } else if (node->left != NULL) {
            node = node->left;
        } else {
            return node;
        }
    }
}

struct bnode *
bnextleaf(struct bnode *node)
{
    struct bnode *successor;
    if ((successor = bsuccessor(node)) != NULL) {
        return blowestleaf(successor->right);
    }
    return NULL;
}

struct bnode *
bprevleaf(struct bnode *node)
{
    struct bnode *predecessor;
    if ((predecessor = bpredecessor(node)) != NULL) {
        return bhighestleaf(predecessor->left);
    }
    return NULL;
}

void
free_tree(struct bnode *root)
{
    if (!bisinternal(root)) {
        free(root->arc); // could be ->arc or ->bp
        free(root);
        return;
    }
    free_tree(root->left);
    free_tree(root->right);
    free(root->bp); // could be ->arc or ->bp
    free(root);
}
