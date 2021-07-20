#ifndef MAIN_H
#define MAIN_H

#include "fortunes.h"
#include "gradient.h"
#include "config.h"

// length of lines when reading file
#define LINELEN 80
#define OPTSTR "b:o:n:d:hsi:"
#define USAGE_FMT \
    "usage: %s [-s] [-i PATH] [-b t|b] [-o p[r]] [-d c|b|j|s] [-n NUM] [-h]\n"
#define DEFAULT_PROGNAME "voronoi"
#define DEFAULT_INPUTFILE "input"

#endif
