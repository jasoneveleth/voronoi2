#include <stdio.h>
#include "bintree.h"
#include "voronoi.h"
#include <assert.h>
#include <string.h>
#include <math.h>

static void *
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

int
bisinternal(struct bnode *node)
{
    return node != NULL && (node->left != NULL || node->right != NULL);
}

struct bnode *
baddleft(struct bnode *root, void *val)
{
    struct bnode *node = malloc(sizeof(struct bnode));
    node->bp = val; // can be either ->bp or ->arc
    root->left = node;
    node->parent = root;
    return node;
}

struct bnode *
baddright(struct bnode *root, void *val)
{
    struct bnode *node = malloc(sizeof(struct bnode));
    node->bp = val; // can be either ->bp or ->arc
    root->right = node;
    node->parent = root;
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

float
fsqrt(float x)
{
    return (float)sqrt((double)x);
}

static inline void
quadraticFormula(float a, float b, float c, float *smaller, float *larger)
{
    float x1 = (-b - fsqrt(b * b - 4 * a * c)) / (2 * a);
    float x2 = (-b + fsqrt(b * b - 4 * a * c)) / (2 * a);
    *smaller = x1 > x2 ? x1 : x2;
    *larger = x1 > x2 ? x2 : x1;
}

point
intersect_parabolas(float sweepline, point parabolas[2])
{
    point p1 = parabolas[0]; // parabola on the left of bp
    point p2 = parabolas[1]; // parabola on the right of bp
    float l = sweepline;

    // TODO sweep line and points are smae height

    float TWO = (float)2; // avoid double promotion
    float ONE = (float)1; // avoid double promotion
    float b = (p2.x) / (p2.y - l) - (p1.x) / (p1.y - l);
    float c = (p1.x * p1.x + p1.y * p1.y - l * l) / (TWO * (p1.y - l))
              - (p2.x * p2.x + p2.y * p2.y - l * l) / (TWO * (p2.y - l));

    // TODO mutliple points have same y value

    float a = ONE / (TWO * (p1.y - l)) - ONE / (TWO * (p2.y - l));

    float x1, x2;
    quadraticFormula(a, b, c, &x1, &x2); // we know x1 < x2
    float y1 =
        (ONE / (TWO * (p1.y - l)))
        * (x1 * x1 - TWO * p1.x * x1 + p1.x * p1.x + p1.y * p1.y - l * l);
    float y2 =
        (ONE / (TWO * (p1.y - l)))
        * (x2 * x2 - TWO * p1.x * x2 + p1.x * p1.x + p1.y * p1.y - l * l);
    point left = {x1, y1};
    point right = {x2, y2};

    int parabola_order_is_old_then_new = p1.y > p2.y;
    if (parabola_order_is_old_then_new)
        return left;
    else
        return right;
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
    printf("pred for: %p\n", (void *)node);
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

static void
print_tree(struct bnode *root)
{
    if (root == NULL) return;
    printf("node: %lx, parent: %lx, left: %lx, right: %lx\n",
           (long)root / 16 % (16 * 16 * 16),
           (long)root->parent / 16 % (16 * 16 * 16),
           (long)root->left / 16 % (16 * 16 * 16),
           (long)root->right / 16 % (16 * 16 * 16));
    print_tree(root->left);
    print_tree(root->right);
}

struct bnode *
bsuccessor(struct bnode *node)
{
    printf("succ for: %p\n", (void *)node);
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

static struct bnode *
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

static struct bnode *
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
