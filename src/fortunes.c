#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fortunes.h"

inline void
free_edgelist(struct edgelist *e)
{
    for (int i = 0; i < e->nedges; i++) { free(e->edges[i]); }
    free(e->edges);
}

static inline void
fill_queue(struct heap *heap, point *sites, int32_t nsites)
{
    for (int i = 0; i < nsites; i++) {
        struct event *e = malloc(sizeof(struct event));
        e->site = sites[i];
        e->kind = 's';
        hinsert(heap, e, e->site.y);
    }
}

static inline struct bp *
new_bp(struct halfedge *edge, point lparabola, point rparabola)
{
    struct bp *breakpoint = malloc(sizeof(struct bp));
    breakpoint->edge = edge;
    breakpoint->sites[0] = lparabola;
    breakpoint->sites[1] = rparabola;
    return breakpoint;
}

static inline struct arc *
new_arc(point site, struct hnode *event)
{
    struct arc *arc = malloc(sizeof(struct arc));
    arc->circle_event = event;
    arc->site = site;
    return arc;
}

static inline int
converge(struct bp *b1, struct bp *b2)
{
    // check if determinant of vectors from site1 -> site2, <x1,y1>
    // and site2 -> site3, <x2,y2> is negative
    float x1 = b1->sites[1].x - b1->sites[0].x;
    float y1 = b1->sites[1].y - b1->sites[0].y;
    float x2 = b2->sites[1].x - b2->sites[0].x;
    float y2 = b2->sites[1].y - b2->sites[0].y;
    return x1 * y2 - y1 * x2 < 0;
}

static inline void
check_new_circle(struct heap *heap,
                 struct bnode *lnode,
                 struct bnode *mnode,
                 struct bnode *rnode)
{
    if (lnode == NULL || rnode == NULL) return;
    if (converge(bpredecessor(mnode)->bp, bsuccessor(mnode)->bp)) {
        point p =
            circleBottom(lnode->arc->site, mnode->arc->site, rnode->arc->site);
        struct event *e = malloc(sizeof(struct event));
        e->kind = 'c';
        e->circle_bottom = p;
        e->leaf = mnode;
        struct hnode *event_hnode = hinsert(heap, e, e->circle_bottom.y);
        mnode->arc->circle_event = event_hnode;
    }
}

static inline void
new_edge(struct edgelist *edgelist, struct halfedge **h1, struct halfedge **h2)
{
    (*h1) = malloc(sizeof(struct halfedge));
    (*h2) = malloc(sizeof(struct halfedge));
    (*h1)->twin = (*h2);
    (*h2)->twin = (*h1);

    (*h1)->origin.x = 69;
    (*h1)->origin.y = 69;
    (*h2)->origin.x = 69;
    (*h2)->origin.y = 69;

    if (edgelist->nedges >= edgelist->allocated) {
        edgelist->allocated *= 2;
        size_t new_size =
            sizeof(struct halfedge *) * (size_t)(edgelist->allocated);
        edgelist->edges = realloc(edgelist->edges, new_size);
    }
    edgelist->edges[edgelist->nedges] = (*h1);
    edgelist->nedges++;
    edgelist->edges[edgelist->nedges] = (*h2);
    edgelist->nedges++;
}

static inline void
remove_false_alarm(struct heap *heap, struct arc *arc)
{
    if (arc->circle_event) {
        // remove false alarm
        struct event *old_event = hremove(heap, arc->circle_event);
        arc->circle_event = NULL;
        free(old_event);
    }
}

static inline void
add_subtree(struct bnode **lnode,
            struct bnode **bp,
            struct bnode **mnode,
            struct bnode **rnode,
            struct bnode **old_node,
            point old_site,
            struct halfedge *edge,
            struct halfedge *edgetwin,
            point event_site)
{
    struct arc *larc = new_arc(old_site, NULL);
    struct bp *lbp = new_bp(edgetwin, event_site, old_site);
    struct arc *marc = new_arc(event_site, NULL);
    struct bp *rbp = new_bp(edge, old_site, event_site);
    struct arc *rarc = new_arc(old_site, NULL);
    (*old_node)->bp = rbp;
    *lnode = baddleft(*old_node, larc);
    *bp = baddright(*old_node, lbp);
    *mnode = baddleft(*bp, marc);
    *rnode = baddright(*bp, rarc);
}

static inline void
handle_site_event(struct event *event,
                  struct bnode *root,
                  struct heap *heap,
                  struct edgelist *edgelist)
{
    if (root->arc == NULL) {
        root->arc = new_arc(event->site, NULL);
        return;
    }

    struct bnode *old_node = bfindarc(root, event->site);
    struct arc *old_arc = old_node->arc;
    point old_site = old_arc->site;
    remove_false_alarm(heap, old_arc);
    free(old_arc);

    // make edges
    struct halfedge *edge, *edgetwin;
    new_edge(edgelist, &edge, &edgetwin);

    struct bnode *larc, *bp, *marc, *rarc;
    add_subtree(&larc, &bp, &marc, &rarc, &old_node, old_site, edge, edgetwin,
                event->site);

    check_new_circle(heap, bprevleaf(larc), larc, marc);
    check_new_circle(heap, marc, rarc, bnextleaf(rarc));
}

static inline void
handle_circle_event(struct event *event,
                    // struct bnode *root,
                    struct heap *heap,
                    struct edgelist *edgelist)
{
    struct bnode *leaf = event->leaf;
    struct bnode *parent = leaf->parent;
    struct bnode *nextleaf = bnextleaf(leaf);
    struct bnode *prevleaf = bprevleaf(leaf);
    point center = circle_center(prevleaf->arc->site, leaf->arc->site,
                                 nextleaf->arc->site);
    int leaf_is_left_child = parent->left == leaf;
    struct bnode *other_child =
        leaf_is_left_child ? parent->right : parent->left;
    // this will be the not-parent bp
    struct bp *other_bp =
        leaf_is_left_child ? bpredecessor(leaf)->bp : bsuccessor(leaf)->bp;

    // fix remaining bp (the not-parent one) with the correct two sites
    other_bp->sites[leaf_is_left_child] = parent->bp->sites[leaf_is_left_child];

    other_bp->edge->origin = center;
    parent->bp->edge->origin = center;

    // replace parent with 'other child' of parent (i.e. the one that's not
    // 'leaf')
    free(bremove(leaf));
    free(parent->bp);
    parent->arc = other_child->arc;
    parent->left = other_child->left;
    parent->right = other_child->right;
    if (parent->left) parent->left->parent = parent;
    if (parent->right) parent->right->parent = parent;
    // refresh the leave pointers in case they were the other child
    if (nextleaf == other_child) nextleaf = parent;
    if (prevleaf == other_child) prevleaf = parent;
    free(other_child);

    remove_false_alarm(heap, nextleaf->arc);
    remove_false_alarm(heap, prevleaf->arc);

    struct halfedge *edge, *edgetwin;
    new_edge(edgelist, &edge, &edgetwin);
    edgetwin->origin = center;
    other_bp->edge = edge;

    // TODO set pointers between the two new halfedges, and two (maybe 4) old
    // halfedges right
    //                 ==
    // Set the pointers between them appropriately. Attach the three
    // new records to the half-edge records that end at the vertex.

    check_new_circle(heap, bprevleaf(prevleaf), prevleaf, nextleaf);
    check_new_circle(heap, prevleaf, nextleaf, bnextleaf(nextleaf));
}

static inline void
intersect_lines(point *dest, point p1, point p2, point p3, point p4)
{
    // https://en.wikipedia.org/wiki/Line–line_intersection
    float x1 = p1.x, y1 = p1.y;
    float x2 = p2.x, y2 = p2.y;
    float x3 = p3.x, y3 = p3.y;
    float x4 = p4.x, y4 = p4.y;
    float D = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    float subexpr = x1 * y2 - y1 * x2;
    float subexpr2 = x3 * y4 - y3 * x4;
    dest->x = (subexpr * (x3 - x4) - subexpr2 * (x1 - x2)) / D;
    dest->y = (subexpr * (y3 - y4) - subexpr2 * (y1 - y2)) / D;
}

static inline void
bound_edges(struct halfedge *edge)
{
    point *dest = &edge->origin;
    point *twin = &edge->twin->origin;
    point bot_left = {0, 0};
    point top_left = {0, 1};
    point bot_right = {1, 0};
    point top_right = {1, 1};
    if (edge->origin.x < 0)
        intersect_lines(dest, bot_left, top_left, *dest, *twin);
    if (edge->origin.x > 1)
        intersect_lines(dest, bot_right, top_right, *dest, *twin);
    if (edge->origin.y < 0)
        intersect_lines(dest, bot_left, bot_right, *dest, *twin);
    if (edge->origin.y > 1)
        intersect_lines(dest, top_left, top_right, *dest, *twin);
}

static inline void
compute_bounding_box(struct bnode *root, struct edgelist *edgelist)
{
    float l = (float)-1.4143; // hard coded -sqrt(2), (diagonal of bounding box)
    struct bnode *node = bgetmin(root);
    while ((node = bsuccessor(node)) != NULL) { // finish edges
        if (!bisinternal(node)) continue;
        point intersection = intersect_parabolas(l, node->bp->sites);
        node->bp->edge->origin = intersection;
    }
    for (int i = 0; i < edgelist->nedges; i++) {
        struct halfedge *edge = edgelist->edges[i];
        point origin = edge->origin;
        if (origin.x < 0 || origin.x > 1 || origin.y < 0 || origin.y > 1)
            bound_edges(edge);
    }
}

void
fortunes(point *sites, int32_t nsites, struct edgelist *edgelist)
{
    struct heap *heap = init_heap();
    struct bnode *root = malloc(sizeof(struct bnode));
    root->parent = NULL;
    root->left = NULL;
    root->right = NULL;
    root->arc = NULL;

    fill_queue(heap, sites, nsites);
    while (!hempty(heap)) {
        struct event *e = hremove_max(heap);
        if (e->kind == 's') {
            handle_site_event(e, root, heap, edgelist);
        } else {
            handle_circle_event(e, heap, edgelist);
        }
        free(e);
    }
    compute_bounding_box(root, edgelist);
    free_heap(heap);
    free_tree(root);
}

/* vim: set ft=c.makemaps: */
