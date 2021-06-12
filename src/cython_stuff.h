#ifndef VORONOI_H
#define VORONOI_H

struct arrays {
    float *linesegs_to_be_cast;
    float *sites_to_be_cast;
    float *perimeter;
    float *objective_function;
    float *char_max_length;
    float *char_min_length;
};

void simple_diagram(float *, int, float *, int);
void gradient_descent(struct arrays, int, int);
void init_options(const char *filepath,
                  unsigned char obj,
                  unsigned char descent,
                  unsigned char gradient,
                  unsigned char boundary,
                  float alpha,
                  float repel_coeff,
                  int ntrials,
                  float jiggle);

#endif
