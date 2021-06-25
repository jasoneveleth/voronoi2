#ifndef CONFIG_H
#define CONFIG_H

enum __attribute__((packed)) objective_function
{
    PERIMETER = 1,
    REPULSION = 2,
    // powers of 2, becuase it's a set
};

enum __attribute__((packed)) descent_method
{
    CONSTANT_ALPHA,
    BARZIILAI,
    CONJUGATE
};

enum __attribute__((packed)) gradient_method
{
    FINITE_DIFFERENCE,
};

enum __attribute__((packed)) boundary_condition
{
    BOUNCE,
    TORUS,
};

// all need to be nonzero when used
struct options {
    const char *filepath;
    enum objective_function obj;
    enum descent_method descent;
    enum gradient_method gradient;
    enum boundary_condition boundary;
    float alpha;
    float repel_coeff;
    float jiggle;
    unsigned long ntrials;
};

struct arrays {
    float *linesegs_to_be_cast;
    float *sites_to_be_cast;
    float *perimeter;
    float *objective_function;
    float *char_max_length;
    float *char_min_length;
};

#endif
