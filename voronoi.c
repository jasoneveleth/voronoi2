#include <stdio.h>
#include <string.h>
#include "bintree.h"
#include "voronoi.h"
#include "heap.h"

// length of lines when reading file
#define LINELEN 80

static void
print_edgelist(struct edgelist *edgelist)
{
    for (int i = 0; i < edgelist->nedges; i += 2) {
        printf("(%f,%f),(%f,%f)\t",
               (double)edgelist->edges[i]->origin.x,
               (double)edgelist->edges[i]->origin.y,
               (double)edgelist->edges[i + 1]->origin.x,
               (double)edgelist->edges[i + 1]->origin.y);
    }
}

#ifdef DEBUG
static void
print_edge(halfedge *e)
{
    printf("new edges: edge: %p, twin: %p\n", (void *)e, (void *)e->twin);
}
#endif

static void
free_edgelist(struct edgelist *e) {
    for (int i = 0; i < e->nedges; i++) {
        free(e->edges[i]);
    }
    free(e->edges);
}

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

static void
new_edge(struct edgelist *edgelist, struct halfedge **h1, struct halfedge **h2)
{
    struct halfedge *e1 = malloc(sizeof(struct halfedge));
    struct halfedge *e2 = malloc(sizeof(struct halfedge));
    e1->twin = e2;
    e2->twin = e1;

    e1->origin.x = 69;
    e1->origin.y = 69;
    e2->origin.x = 69;
    e2->origin.y = 69;

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
                  struct bnode *root,
                  struct heap *heap,
                  struct edgelist *edgelist)
{
    point new_site = event->site;

    if (root->arc == NULL) {
        root->arc = new_arc(new_site, NULL);
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

    // add subtree (ordered here by preorder traversal)
    old_node->bp = new_bp(edge, old_site, new_site);
    struct bnode *larc = baddleft(old_node, new_arc(old_site, NULL));
    struct bnode *bp =
        baddright(old_node, new_bp(edgetwin, new_site, old_site));
    struct bnode *marc = baddleft(bp, new_arc(new_site, NULL));
    struct bnode *rarc = baddright(bp, new_arc(old_site, NULL));

    check_new_circle(heap, bprevleaf(larc), larc, marc);
    check_new_circle(heap, marc, rarc, bnextleaf(rarc));
}

static void
handle_circle_event(struct event *event,
                    struct heap *heap,
                    struct edgelist *edgelist)
{
    struct bnode *leaf = event->leaf;
    struct bnode *parent = leaf->parent;
    struct bnode *nextleaf = bnextleaf(leaf);
    struct bnode *prevleaf = bprevleaf(leaf);
    point center = circle_center(
        prevleaf->arc->site, leaf->arc->site, nextleaf->arc->site);
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
    parent->left->parent = parent;
    parent->right->parent = parent;
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
compute_bounding_box(struct bnode *root, struct edgelist *edgelist)
{
    float l = (float)-1.4143; // hard coded -sqrt(2), (diagonal of bounding box)
    struct bnode *node = bgetmin(root);
    while ((node = bsuccessor(node)) != NULL) { // finish edges
        if (!bisinternal(node)) continue;
        point intersection = intersect_parabolas(l, node->bp->sites);
        node->bp->edge->origin = intersection;
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

static inline void
read_sites_from_file(const char *path, point **arr_ptr, int32_t *length)
{
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "path not valid: %s\n", path);
        exit(1);
    }
    char line[LINELEN];
    point *arr = malloc(2 * sizeof(point));
    int32_t allocated = 2;
    int nsites;
    for (nsites = 0; fgets(line, LINELEN, file) != NULL; nsites++) {
        if (nsites >= allocated) {
            allocated *= 2;
            arr = realloc(arr, sizeof(point) * (size_t)(allocated));
        }
        char *first = strtok(line, ", \t");
        char *second = strtok(NULL, ", \t");
        arr[nsites].x = strtof(first, NULL);
        arr[nsites].y = strtof(second, NULL);
    }
    fclose(file);
    arr = realloc(arr, (size_t)nsites * sizeof(point));
    *length = nsites;
    *arr_ptr = arr;
}

static void
print_sites(point *sites, int32_t length)
{
    for (int i = 0; i < length; i++)
        printf("(%f,%f)\t", (double)sites[i].x, (double)sites[i].y);
    printf("\n");
}

static void
init_edgelist(struct edgelist *e)
{
    e->nedges = 0;
    e->allocated = 1024;
    // maybe make it an array of structs rather than pointers
    e->edges = malloc((size_t)e->allocated * sizeof(struct halfedge *));
}

int
main(int argc, char **argv)
{
    struct edgelist e;
    init_edgelist(&e);
    if (argc == 1) {
        point sites[3] = {
            {(float)0.875, (float)0.169},
            {(float)0.852, (float)0.792},
            {(float)0.233, (float)0.434},
        };
        print_sites(sites, 3);
        fortunes(sites, 3, &e);
    } else {
        int32_t nsites = 0;
        point *sites;
        read_sites_from_file(argv[argc - 1], &sites, &nsites);
        print_sites(sites, nsites);
        fortunes(sites, nsites, &e);
        free(sites);
    }
    print_edgelist(&e);
    free_edgelist(&e);
    return 0;
}
