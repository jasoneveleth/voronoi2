#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "edgelist.h"

float
calc_perimeter(struct edgelist *edgelist)
{
    float length = 0;
    for (int i = 0; i < edgelist->nedges; i++) {
        float dx =
            edgelist->edges[i]->origin.x - edgelist->edges[i]->twin->origin.x;
        float dy =
            edgelist->edges[i]->origin.y - edgelist->edges[i]->twin->origin.y;
        float d = sqrtf(dx * dx + dy * dy);
        length += d;
    }
    return (length / 2) + 4;
}

void
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

void
copy_edges(struct edgelist *edgelist, point *dest)
{
    for (int i = 0; i < edgelist->nedges; i++) {
        // multiply by 2 because: 1 edge = 2 points
        dest[i * 2].x = edgelist->edges[i]->origin.x;
        dest[i * 2].y = edgelist->edges[i]->origin.y;
        dest[i * 2 + 1].x = edgelist->edges[i]->twin->origin.x;
        dest[i * 2 + 1].y = edgelist->edges[i]->twin->origin.y;
    }
}

void
init_edgelist(struct edgelist *e)
{
    e->nedges = 0;
    e->allocated = 1024;
    // maybe make it an array of structs rather than pointers
    e->edges = malloc((size_t)e->allocated * sizeof(struct halfedge *));
}

void
calc_char_length(struct edgelist *edgelist, float *max, float *min)
{
    *max = 0.0f;
    *min = 1.0f / 0.0f;
    for (int i = 0; i < edgelist->nedges; i++) {
        float x1 = edgelist->edges[i]->origin.x;
        float y1 = edgelist->edges[i]->origin.y;
        float x2 = edgelist->edges[i]->twin->origin.x;
        float y2 = edgelist->edges[i]->twin->origin.y;
        float dist = sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
        if (dist > *max) *max = dist;
        if (dist < *min) *min = dist;
    }
}

void
print_edgelist(struct edgelist *edgelist)
{
    for (int i = 0; i < edgelist->nedges; i += 2) {
        printf("(%f,%f),(%f,%f),\t", (double)edgelist->edges[i]->origin.x,
               (double)edgelist->edges[i]->origin.y,
               (double)edgelist->edges[i + 1]->origin.x,
               (double)edgelist->edges[i + 1]->origin.y);
    }
    printf("\n");
}

void
print_edge(struct halfedge *e)
{
    printf("new edges: edge: %p, twin: %p\n", (void *)e, (void *)e->twin);
}

void
print_sites(const point *const sites, const int32_t length)
{
    int len = 0;
    for (int i = 0; i < length; i++) {
        printf("(%f,%f),\t", (double)sites[i].x, (double)sites[i].y);
        if ((len += 25) > 80) {
            len = 0;
            printf("\n");
        }
    }
    printf("\n");
}
