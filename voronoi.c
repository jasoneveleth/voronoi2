#include "bintree.h"
#include "voronoi.h"
#include "heap.h"
#include <stdio.h>

#ifdef DEBUG
static void
print_edgelist(edgelist *edgelist)
{
    for (int i = 0; i < edgelist->nedges; i++) {
        printf("edge: address: %p, origin: (%f, %f), twin: %p\n",
               (void *)edgelist->edges[i],
               (double)edgelist->edges[i]->origin.x,
               (double)edgelist->edges[i]->origin.y,
               (void *)edgelist->edges[i]->twin);
    }
}

static void
print_edge(halfedge *e)
{
    printf("new edges: edge: %p, twin: %p\n", (void *)e, (void *)e->twin);
}
#endif

static void
fill_queue(struct heap *heap, point *sites, int32_t nsites)
{
    for (int i = 0; i < nsites; i++) {
        struct event *e = malloc(sizeof(struct event));
        e->site = sites[i];
        e->kind = 's';
        hinsert(heap, e, e->site.y);
    }
}

static struct bp *
new_bp(struct halfedge *edge, point lparabola, point rparabola)
{
    struct bp *breakpoint = malloc(sizeof(struct bp));
    breakpoint->edge = edge;
    breakpoint->sites[0] = lparabola;
    breakpoint->sites[1] = rparabola;
    return breakpoint;
}

static struct arc *
new_arc(point site, struct hnode *event)
{
    struct arc *arc = malloc(sizeof(struct arc));
    arc->circle_event = event;
    arc->site = site;
    return arc;
}

static int
converge(struct bp *b1, struct bp *b2)
{
    // check if determinant of vectors from site1 -> site2, <x1,y1>
    // and site2 -> site3, <x2,y2> is negative
    float x1 = b1->sites[1].x - b1->sites[0].x;
    float y1 = b1->sites[1].y - b1->sites[0].y;
    float x2 = b2->sites[1].x - b2->sites[0].x;
    float y2 = b2->sites[1].y - b2->sites[0].y;
#ifdef DEBUG
    printf("sites: (%f, %f), (%f, %f), (%f, %f)\n",
           (double)b1->sites[0].x,
           (double)b1->sites[0].y,
           (double)b1->sites[1].x,
           (double)b1->sites[1].y,
           (double)b2->sites[1].x,
           (double)b2->sites[1].y);
    printf("vectors: <%f, %f>, <%f, %f>\n",
           (double)x1,
           (double)y1,
           (double)x2,
           (double)y2);
    printf("det: %f\n", (double)(x1 * y2 - y1 * x2));
#endif
    return x1 * y2 - y1 * x2 < 0;
}

static point
circle_center(point a, point b, point c)
{
    float ONE = (float)1;
    float d = 2 * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));
    float x = (ONE / d)
              * ((a.x * a.x + a.y * a.y) * (b.y - c.y)
                 + (b.x * b.x + b.y * b.y) * (c.y - a.y)
                 + (c.x * c.x + c.y * c.y) * (a.y - b.y));
    float y = (ONE / d)
              * ((a.x * a.x + a.y * a.y) * (c.x - b.x)
                 + (b.x * b.x + b.y * b.y) * (a.x - c.x)
                 + (c.x * c.x + c.y * c.y) * (b.x - a.x));
    point p = {x, y};
    return p;
}

static point
circleBottom(point a, point b, point c)
{
    point p = circle_center(a, b, c);
    float r = fsqrt((a.x - p.x) * (a.x - p.x) + (a.y - p.y) * (a.y - p.y));
    p.y -= r;
    return p;
}

static void
check_new_circle(struct bintree *tree,
                 struct heap *heap,
                 struct bnode *lnode,
                 struct bnode *mnode,
                 struct bnode *rnode)
{
    if (lnode == NULL || rnode == NULL) return;
    if (converge(bpredecessor(tree, mnode)->bp, bsuccessor(tree, mnode)->bp)) {
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

static void
new_edge(struct edgelist *edgelist, struct halfedge **h1, struct halfedge **h2)
{
    struct halfedge *e1 = malloc(sizeof(struct halfedge));
    struct halfedge *e2 = malloc(sizeof(struct halfedge));
    e1->twin = e2;
    e2->twin = e1;
    if (edgelist->nedges >= edgelist->allocated) {
        edgelist->allocated *= 2;
        edgelist->edges =
            realloc(edgelist->edges,
                    sizeof(struct halfedge *) * (size_t)(edgelist->allocated));
    }
    edgelist->edges[edgelist->nedges] = e1;
    edgelist->nedges++;
    edgelist->edges[edgelist->nedges] = e2;
    edgelist->nedges++;
    *h1 = e1;
    *h2 = e2;
}

static inline void
remove_false_alarm(struct heap *heap, struct arc *old_arc)
{
    if (old_arc->circle_event) {
        // remove false alarm
        struct event *old_event = hremove(heap, old_arc->circle_event);
        free(old_event);
    }
}

static void
handle_site_event(struct event *event,
                  struct bintree *tree,
                  struct heap *heap,
                  struct edgelist *edgelist)
{
    point new_site = event->site;
    free(event);

    if (bempty(tree)) {
        binsert(tree, new_arc(new_site, NULL), 1);
        return;
    }

    struct bnode *old_node = bfindarc(tree, event->site);
    int32_t old_index = old_node->index;
    struct arc *old_arc = bremove(tree, old_node);
    point old_site = old_arc->site;
    remove_false_alarm(heap, old_arc);
    free(old_arc);

    // make edges
    struct halfedge *edge, *edgetwin;
    new_edge(edgelist, &edge, &edgetwin);

    // add subtree (ordered here by preorder traversal)
    struct bnode *root =
        binsert(tree, new_bp(edge, old_site, new_site), old_index);
    struct bnode *larc =
        binsert(tree, new_arc(old_site, NULL), root->index * 2);
    struct bnode *bp = binsert(
        tree, new_bp(edgetwin, new_site, old_site), root->index * 2 + 1);
    struct bnode *marc = binsert(tree, new_arc(new_site, NULL), bp->index * 2);
    struct bnode *rarc =
        binsert(tree, new_arc(old_site, NULL), bp->index * 2 + 1);

    check_new_circle(tree, heap, bprevleaf(tree, larc), larc, marc);
    check_new_circle(tree, heap, marc, rarc, bnextleaf(tree, rarc));
}

static void
handle_circle_event(struct event *event,
                    struct bintree *tree,
                    struct heap *heap,
                    struct edgelist *edgelist)
{
    for (int i = 0; i < 40; i++) printf("[%d] %p\n", i, (void *)tree->arr[i]);
    printf("[%d] %p\n", event->leaf->index, (void *)event->leaf);
    struct bnode *leaf = event->leaf;
    struct bnode **parent = &tree->arr[leaf->index / 2];
    struct bnode *nextleaf = bnextleaf(tree, leaf);
    struct bnode *prevleaf = bprevleaf(tree, leaf);
    point center = circle_center(
        prevleaf->arc->site, leaf->arc->site, nextleaf->arc->site);
    int leaf_is_left_child = leaf->index % 2 == 0;
    int offset = leaf_is_left_child * 2 - 1; // from leaf to other_child
    struct bnode **other_child = &tree->arr[leaf->index + offset];
    // this will be the not-parent bp
    struct bp *other_bp = leaf_is_left_child ? bpredecessor(tree, leaf)->bp
                                             : bsuccessor(tree, leaf)->bp;

    // 1. fix remaining bp (the not-parent one) with the correct two sites
    other_bp->sites[leaf_is_left_child] =
        (*parent)->bp->sites[leaf_is_left_child];

    // 2. replace parent with 'other child' of parent (i.e. the one that's not
    // 'leaf')
    free((*parent)->bp);
    (*parent)->arc = (*other_child)->arc;
    *other_child = NULL;

    remove_false_alarm(heap, nextleaf->arc);
    remove_false_alarm(heap, prevleaf->arc);

    struct bp *parent_bp = bremove(tree, *parent);
    struct arc *arc = bremove(tree, leaf);

    other_bp->edge->twin->origin = center;
    parent_bp->edge->twin->origin = center;

    free(parent_bp);
    free(arc);

    struct halfedge *edge, *edgetwin;
    new_edge(edgelist, &edge, &edgetwin);
    edgetwin->origin = center;
    other_bp->edge = edge;

    // TODO set pointers between the two new halfedges, and two (maybe 4) old
    // halfedges right
    //                 ==
    // Set the pointers between them appropriately. Attach the three
    // new records to the half-edge records that end at the vertex.

    check_new_circle(tree, heap, bprevleaf(tree, prevleaf), prevleaf, nextleaf);
    check_new_circle(tree, heap, prevleaf, nextleaf, bnextleaf(tree, nextleaf));
    for (int i = 0; i < 40; i++) printf("[%d] %p\n", i, (void *)tree->arr[i]);
}

static void
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

static void
compute_bounding_box(struct bintree *tree, struct edgelist *edgelist)
{
    float l = (float)-1.4143; // hard coded -sqrt(2), (diagonal of bounding box)
    struct bnode *node = bgetmin(tree, tree->arr[1]);
    while ((node = bsuccessor(tree, node)) != NULL) { // finish edges
        if (!bisinternal(tree, node)) continue;
        point intersection = intersect_parabolas(l, node->bp->sites);
        node->bp->edge->twin->origin = intersection;
    }
    for (int i = 0; i < edgelist->nedges; i++) { // bound edges
        struct halfedge *edge = edgelist->edges[i];
        if (edge->origin.x < 0) {
            point bottom = {0, 0};
            point top = {0, 1};
            intersect_lines(
                &edge->origin, bottom, top, edge->origin, edge->twin->origin);
        }
        if (edge->origin.y < 0) {
            point left = {0, 0};
            point right = {1, 0};
            intersect_lines(
                &edge->origin, left, right, edge->origin, edge->twin->origin);
        }
        if (edge->origin.x > 1) {
            point bottom = {1, 0};
            point top = {1, 1};
            intersect_lines(
                &edge->origin, bottom, top, edge->origin, edge->twin->origin);
        }
        if (edge->origin.y > 1) {
            point left = {0, 1};
            point right = {1, 1};
            intersect_lines(
                &edge->origin, left, right, edge->origin, edge->twin->origin);
        }
    }
}

void
fortunes(point *sites, int32_t nsites, struct edgelist *edgelist)
{
    struct heap *heap = init_heap();
    struct bintree *tree = init_tree();

    fill_queue(heap, sites, nsites);
    while (!hempty(heap)) {
        struct event *e = hremove_max(heap);
        if (e->kind == 's') {
            handle_site_event(e, tree, heap, edgelist);
        } else {
            handle_circle_event(e, tree, heap, edgelist);
        }
    }
    compute_bounding_box(tree, edgelist);
}

int
main()
{
#define NSITES 3
    point sites[NSITES] = {
        {(float)0.875, (float)0.169},
        {(float)0.852, (float)0.792},
        {(float)0.233, (float)0.434},
    };
    struct edgelist e;
    e.nedges = 0;
    e.allocated = 1024;
    e.edges = malloc((size_t)e.allocated * sizeof(struct halfedge *));
    // maybe make it an array of structs rather than pointers
    fortunes(sites, NSITES, &e);
    for (int i = 0; i < NSITES; i++)
        printf("(%f, %f), ", (double)sites[i].x, (double)sites[i].y);
    printf("\n");
    for (int i = 0; i < e.nedges; i++)
        printf("(%f, %f), ",
               (double)e.edges[i]->origin.x,
               (double)e.edges[i]->origin.y);
    printf("\n");
    return 0;
}
