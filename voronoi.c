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
        event *e = malloc(sizeof(event));
        e->site_event.site = sites[i];
        e->site_event.kind = 's';
        hinsert(heap, e, e->site_event.site.y);
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

// TODO wrogn
static int
converge(struct bp *b1, struct bp *b2)
{
    return (int)(b1 - b2) * (int)0;
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
    struct bp *b1 = bpredecessor(tree, mnode)->bp;
    struct bp *b2 = bsuccessor(tree, mnode)->bp;
    if (converge(b1, b2)) {
        point p =
            circleBottom(lnode->arc->site, mnode->arc->site, rnode->arc->site);
        event *e = malloc(sizeof(event));
        e->circle_event.kind = 'c';
        e->circle_event.lowest_point = p;
        e->circle_event.arc = mnode->arc;
        struct hnode *event_hnode =
            hinsert(heap, e, e->circle_event.lowest_point.y);
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
    edgelist->edges[edgelist->nedges] = e1;
    edgelist->nedges++;
    edgelist->edges[edgelist->nedges] = e2;
    edgelist->nedges++;
    *h1 = e1;
    *h2 = e2;
}

static void
handle_site_event(event *event,
                  struct bintree *tree,
                  struct heap *heap,
                  struct edgelist *edgelist)
{
    point new_site = event->site_event.site;
    free(event);

    if (bempty(tree)) {
        struct arc *arc = malloc(sizeof(struct arc));
        arc->circle_event = NULL;
        arc->site = new_site;
        binsert(tree, arc, 1);
        return;
    }

    struct bnode *old_node = bfindarc(tree, event->site_event.site);
    struct arc *old_arc = old_node->arc;
    bremove(tree, old_node);
    int32_t old_index = old_node->index;
    point old_site = old_arc->site;
    if (old_arc->circle_event) {
        union event *old_event =
            hremove(heap, old_arc->circle_event); // remove false alarm
        free(old_event);
    }
    free(old_arc);

    // make edges
    struct halfedge *edge, *edgetwin;
    new_edge(edgelist, &edge, &edgetwin);

    // add subtree (ordered here by inorder traversal)
    struct arc *larc = new_arc(old_site, NULL);
    struct bnode *larc_node = binsert(tree, larc, old_index * 2);
    struct bp *lbreakpoint = new_bp(edge, old_site, new_site);
    // bnode *lbp_node = binsert(tree, lbreakpoint, old_index);
    binsert(tree, lbreakpoint, old_index);
    struct arc *marc = new_arc(new_site, NULL);
    struct bnode *marc_node = binsert(tree, marc, (old_index * 2 + 1) * 2);
    struct bp *rbreakpoint = new_bp(edgetwin, new_site, old_site);
    // bnode *rbp_node = binsert(tree, rbreakpoint, old_index*2 + 1);
    binsert(tree, rbreakpoint, old_index * 2 + 1);
    struct arc *rarc = new_arc(old_site, NULL);
    struct bnode *rarc_node = binsert(tree, rarc, (old_index * 2 + 1) * 2 + 1);

    check_new_circle(
        tree, heap, bprevleaf(tree, larc_node), larc_node, marc_node);
    check_new_circle(
        tree, heap, marc_node, rarc_node, bnextleaf(tree, rarc_node));
}

static void
handle_circle_event(event *event,
                    struct bintree *tree,
                    struct heap *heap,
                    struct edgelist *edgelist)
{
    return;
}

// static point
// project(point a, point b) {
//     float adotb = a.x * b.x + a.y * b.y;
//     float bdotb = b.x * b.x + b.y * b.y;
//     point proj = {(adotb / bdotb) * b.x, (adotb / bdotb) * b.y};
//     return proj;
// }

static void
intersect_lines(point *dest,
                float x1,
                float y1,
                float x2,
                float y2,
                float x3,
                float y3,
                float x4,
                float y4)
{
    // https://en.wikipedia.org/wiki/Line–line_intersection
    float D = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    float repeated = x1 * y2 - y1 * x2;
    float repeated2 = x3 * y4 - y3 * x4;
    dest->x = (repeated * (x3 - x4) - repeated2 * (x1 - x2)) / D;
    dest->y = (repeated * (y3 - y4) - repeated2 * (y1 - y2)) / D;
}

static void
compute_bounding_box(struct bintree *tree, struct edgelist *edgelist)
{
    float l = (float)-1.4143; // hard coded -sqrt(2), (diagonal of bounding box)
    struct bnode *node = bgetmin(tree, tree->arr[1]);
    while ((node = bsuccessor(tree, node)) != NULL) {
        if (!bisinternal(tree, node)) continue;
        point intersection = intersect_parabolas(l, node->bp->sites);
        node->bp->edge->twin->origin = intersection;
    }
    for (int i = 0; i < edgelist->nedges; i++) {
        struct halfedge *edge = edgelist->edges[i];
        // this is just copy pasta except for the 4 numbers in the middle
        if (edge->origin.x < 0) {
            intersect_lines(&edge->origin,
                            0,
                            0,
                            0,
                            1,
                            edge->origin.x,
                            edge->origin.y,
                            edge->twin->origin.x,
                            edge->twin->origin.y);
        }
        if (edge->origin.y < 0) {
            intersect_lines(&edge->origin,
                            0,
                            0,
                            1,
                            0,
                            edge->origin.x,
                            edge->origin.y,
                            edge->twin->origin.x,
                            edge->twin->origin.y);
        }
        if (edge->origin.x > 1) {
            intersect_lines(&edge->origin,
                            1,
                            0,
                            1,
                            1,
                            edge->origin.x,
                            edge->origin.y,
                            edge->twin->origin.x,
                            edge->twin->origin.y);
        }
        if (edge->origin.y > 1) {
            intersect_lines(&edge->origin,
                            0,
                            1,
                            1,
                            1,
                            edge->origin.x,
                            edge->origin.y,
                            edge->twin->origin.x,
                            edge->twin->origin.y);
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
        event *e = hremove_max(heap);
        if (e->site_event.kind == 's') {
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
    point first = {(float)0.23, (float)0.53};
    point second = {(float)0.74, (float)0.923};
    point sites[2] = {first, second};
    struct edgelist e;
    e.nedges = 0;
    fortunes(sites, 2, &e);
    printf("(%f, %f), (%f, %f)\n",
           (double)e.edges[0]->origin.x,
           (double)e.edges[0]->origin.y,
           (double)e.edges[1]->origin.x,
           (double)e.edges[1]->origin.y);
    return 0;
}
