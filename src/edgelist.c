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
        length += fsqrt(dx * dx + dy * dy);
    }
    return (length / 2) + 4;
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
