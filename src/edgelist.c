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
