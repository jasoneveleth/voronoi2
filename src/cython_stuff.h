#ifndef VORONOI_H
#define VORONOI_H

struct arrays {
    float *linesegs_to_be_cast;
    float *sites_to_be_cast;
    float *perimeter;
    float *objective_function;
    float *char_length;
};

void simple_diagram(float *, int, float *, int);
void gradient_descent(struct arrays, float, int, int, int);

#endif
