#include <stdlib.h>
#include <stdio.h>
#include "src/geometry.h"

int
main(int argc, char *argv[])
{
    int nsites = atoi(argv[1]);
    int ntrials = atoi(argv[2]);
    // read into this array
    point *linesegs;

    FILE *gnuplotPipe = popen("gnuplot -persistent", "w");
    fputs("set terminal gif animate delay 20\n", gnuplotPipe);
    fputs("set output 'newest.gif'\n", gnuplotPipe);
    fputs(
        "set style line 1 linecolor rgb '#0060ad' linetype 1 linewidth 1"
        "pointsize 0\n",
        gnuplotPipe);

    fputs("set xrange [0:1]\n", gnuplotPipe);
    fputs("set yrange [0:1]\n", gnuplotPipe);
    //  4 becuase we have each line segment on there twice, and its 2 points
    //  in a lineseg
    size_t inner_loop_size = 4 * (3 * nsites - 6);
    for (size_t i = 0; i < ntrials; i++) {
        fputs("plot '-' with linespoints linestyle 1\n", gnuplotPipe);
        for (size_t j = 0; j < inner_loop_size; j += 2) {
            point start = linesegs[i * inner_loop_size + j];
            point end = linesegs[i * inner_loop_size + j + 1];
            fprintf(gnuplotPipe, "%f %f\n%f %f\n\n", (double)start.x,
                    (double)start.y, (double)end.x, (double)end.y);
        }
        fputs("e\n", gnuplotPipe);
    }
    fprintf(gnuplotPipe, "exit");
    pclose(gnuplotPipe);
}
