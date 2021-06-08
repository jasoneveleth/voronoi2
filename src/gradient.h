#ifndef GRADIENT_H
#define GRADIENT_H

#include "edgelist.h"
static const float alpha = (float)3e-3;
// static const float alpha = (float)1e-7; // INTERFACE

float obj_function(point *, struct edgelist *, int);
void update_sites(point *, point *, point *, int);

#endif
