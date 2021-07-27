#ifndef MAIN_H
#define MAIN_H

#include "fortunes.h"
#include "gradient.h"
#include "config.h"

// length of lines when reading file
#define LINELEN 80
#define OPTSTR "b:o:n:d:i:f:hs"
#define EMPH(str) \
    "\033[4m"     \
    "\033[1m" str "\033[0m"
#define USAGE_FMT \
    "usage: %s [-s] [-i PATH] [-b t|b] [-o p[r]] [-d c|b|j|s] [-f DIR] [-n " \
    "NUM] [-h]\n" \
    "\t-s silent mode\n" \
    "\t-i input file\n" \
    "\t-b boundary condition, " EMPH("t") "orus or " EMPH("b") "ounce\n" \
    "\t-o objective function, " EMPH("p") "erimeter and/or " EMPH("r") "epulsion\n" \
    "\t-d descent type, " EMPH("c") "onstant alpha, " EMPH("b") "arzilai borwein, con" EMPH("j") "ugate gradient, " EMPH("s") "teepest descent\n" \
    "\t-f output folder\n" \
    "\t-n number of trials\n" \
    "\t-h show this help message\n"
#define DEFAULT_PROGNAME "voronoi"
#define DEFAULT_INPUTFILE "input"

#endif
