#include <stdio.h>
#include "bintree.h"
#include "voronoi.h"
#include <assert.h>
#include <string.h>
#include <math.h>

#ifdef DEBUG
static void
print_tree(bintree *tree)
{
    for (int i = 0; i < 10; i++) {
        if (tree->arr[i] != NULL) {
            if (bisinternal(tree, tree->arr[i])) {
                printf("(node) index: %d, x: %f, bp\n",
                       tree->arr[i]->index,
                       (double)(tree->arr[i]->bp->sites[0].x);
            } else {
                printf("(node) index: %d, x: %f, arc\n",
                       tree->arr[i]->index,
                       (double)((arc *)tree->arr[i]->attr)->site.x);
            }
        }
    }
}
#endif

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

static void
double_tree_size(struct bintree *tree)
{
    size_t old_size = sizeof(struct bnode *) * tree->allocated;
    size_t new_size = sizeof(struct bnode *) * tree->allocated * 2;
    tree->allocated *= 2;
    tree->arr = realloc_zero(tree->arr, old_size, new_size);
}

int
bisinternal(struct bintree *tree, struct bnode *node)
{
    return node != NULL
           && (tree->arr[node->index * 2] != NULL
               || tree->arr[node->index * 2 + 1] != NULL);
}

struct bintree *
init_tree(void)
{
    struct bintree *tree = malloc(sizeof(struct bintree));
    tree->arr = calloc(INIT_SIZE, sizeof(struct bnode *));
    tree->allocated = INIT_SIZE;
    tree->size = 0;
    return tree;
}

struct bnode *
binsert(struct bintree *tree, void *val, int32_t index)
{
#ifdef DEBUG
    printf("binsert(index: %d)\n", index);
#endif
    struct bnode *node = malloc(sizeof(struct bnode));
    node->bp = val; // can be either ->bp or ->arc
    node->index = index;
    if ((size_t)index * 2 + 1 > tree->allocated) double_tree_size(tree);
    tree->arr[index] = node;
    tree->size++;
    return node;
}

void *
bremove(struct bintree *tree, struct bnode *node)
{
#ifdef DEBUG
    printf("bremove(index: %d)\n", node->index);
#endif
    // we assume node is a leaf
    void *attr = node->arc; // can be either -> arc or ->bp
    tree->arr[node->index] = NULL;
    tree->size--;
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
bfindarc(struct bintree *tree, point site)
{
    struct bnode *node = tree->arr[1];
    float sweepline = site.y;
    while (bisinternal(tree, node)) {
        struct bp *breakpoint = node->bp;
        point intersection = intersect_parabolas(sweepline, breakpoint->sites);
        if (site.x > intersection.x) {
            node = tree->arr[node->index * 2 + 1];
        } else {
            node = tree->arr[node->index * 2];
        }
    }
    return node;
}

int32_t
bempty(struct bintree *tree)
{
    return tree->size == 0;
}

struct bnode *
bpredecessor(struct bintree *tree, struct bnode *node)
{
    if (tree->arr[node->index * 2] == NULL) {
        struct bnode *child = node;
        node = tree->arr[node->index / 2];
        while (node != NULL) {
            if (tree->arr[node->index * 2 + 1] == child) return node;
            child = node;
            node = tree->arr[node->index / 2];
        }
        // this handles if we got to the root as the min
        if (node == tree->arr[1])
            if (tree->arr[node->index * 2 + 1] != child) return NULL;
        return node;
    } else {
        return bgetmax(tree, tree->arr[node->index * 2]);
    }
}

struct bnode *
bgetmin(struct bintree *tree, struct bnode *node)
{
    while (tree->arr[node->index * 2] != NULL) {
        node = tree->arr[node->index * 2];
    }
    return node;
}

struct bnode *
bgetmax(struct bintree *tree, struct bnode *node)
{
    while (tree->arr[node->index * 2 + 1] != NULL) {
        node = tree->arr[node->index * 2 + 1];
    }
    return node;
}

struct bnode *
bsuccessor(struct bintree *tree, struct bnode *node)
{
#ifdef DEBUG
    printf("bsuccessor:\n");
    print_tree(tree);
#endif
    if (tree->arr[node->index * 2 + 1] == NULL) {
        struct bnode *child = node;
        node = tree->arr[node->index / 2];
        while (tree->arr[node->index / 2] != NULL) {
            if (tree->arr[node->index * 2] == child) return node;
            child = node;
            node = tree->arr[node->index / 2];
        }

        // this handles when we came from the last element up to root
        if (node == tree->arr[1])
            if (tree->arr[node->index * 2] != child) return NULL;
        return node;
    } else {
        return bgetmin(tree, tree->arr[node->index * 2 + 1]);
    }
}

static struct bnode *
blowestleaf(struct bintree *tree, struct bnode *node)
{
    while (1) {
        if (tree->arr[node->index * 2] != NULL) {
            node = tree->arr[node->index * 2];
        } else if (tree->arr[node->index * 2 + 1] != NULL) {
            node = tree->arr[node->index * 2 + 1];
        } else {
            return node;
        }
    }
}

static struct bnode *
bhighestleaf(struct bintree *tree, struct bnode *node)
{
    while (1) {
        if (tree->arr[node->index * 2 + 1] != NULL) {
            node = tree->arr[node->index * 2 + 1];
        } else if (tree->arr[node->index * 2] != NULL) {
            node = tree->arr[node->index * 2];
        } else {
            return node;
        }
    }
}

struct bnode *
bnextleaf(struct bintree *tree, struct bnode *node)
{
    struct bnode *successor;
    if ((successor = bsuccessor(tree, node)) != NULL) {
        return blowestleaf(tree, tree->arr[successor->index * 2 + 1]);
    }
    return NULL;
}

struct bnode *
bprevleaf(struct bintree *tree, struct bnode *node)
{
    struct bnode *predecessor;
    if ((predecessor = bpredecessor(tree, node)) != NULL) {
        return bhighestleaf(tree, tree->arr[predecessor->index * 2]);
    }
    return NULL;
}
