import numpy as np
import matplotlib.collections
import matplotlib.animation
import matplotlib.pyplot as plt
import sys
from time import time
from random import random
import argparse

start = time()
np.set_printoptions(threshold=sys.maxsize)

def log_time(string):
    global start
    myprint(string)
    myprint(f"elapsed: {(((time() - start)*1000)//1) / 1000} secs\n")
    start = time()

def setup_ax(ax, title, xlim, ylim):
    ax.set_title(title)
    ax.set_xlim(xlim[0], xlim[1])
    ax.set_ylim(ylim[0], ylim[1])

def render_animation(edges, sites, perimeters, objectivefunctions, char_max_length, char_min_length, edgedist):
    """ perimeters : numpy arr (n, 1)
        sites : numpy arr (n, m, 2)
        edges : numpy arr (n, 3*m-6, 2, 2)
    """
    nframes = sites.shape[0]

    fig = plt.figure(figsize=(16, 10))
    # The position of the <edge> of the subplots, as a fraction of the figure width.
    # The width/height of the padding between subplots, as a fraction of the average Axes width/height
    fig.subplots_adjust(left=0.03, bottom=0.03, right=0.97, top=0.97, wspace=0.3, hspace=0.3)
    # (nrows, ncols, start_end_indices)
    perimeter_ax = fig.add_subplot(421)
    char_len_ax = fig.add_subplot(422)
    earth_mover_ax = fig.add_subplot(423)
    edge_dist_ax = fig.add_subplot(424)
    objectivefunction_ax = fig.add_subplot(425)
    diagram_ax = fig.add_subplot(4, 2, (6, 8), aspect='equal')

    setup_ax(perimeter_ax, 'perimeter', (0, nframes), (0, (4/3)*np.max(perimeters)))
    perimeter_line, = perimeter_ax.plot([], [], lw=3) # the comma unpacks the tuple

    setup_ax(diagram_ax, 'voronoi diagram', (0, 1), (0, 1))
    edge_line_coll = matplotlib.collections.LineCollection(())
    diagram_ax.add_collection(edge_line_coll)
    sites_line, = diagram_ax.plot([], [], 'ro')

    setup_ax(char_len_ax, 'longest edge and shortest edge (characteristic length)', (0, nframes), (0, (4/3)*np.max(char_max_length)))
    char_len_max_line, = char_len_ax.plot([], [], lw=3)
    char_len_min_line, = char_len_ax.plot([], [], lw=3)

    setup_ax(earth_mover_ax, 'earth mover distance', (0, nframes), (0,1))

    setup_ax(edge_dist_ax, 'edge distribution', (0, 1.4143), (0, np.max(edgedist) * (4/3)))
    nbars = int(sites.shape[1] * 1.4143)
    x = np.linspace(0, 1.4143, num=nbars, endpoint=False)
    edge_dist_bars = edge_dist_ax.bar(x, edgedist[0], width=(1/sites.shape[1]), align='edge')

    setup_ax(objectivefunction_ax, 'objective function', (0, nframes), (0, (4/3)*np.max(objectivefunctions)))
    objectivefunction_line, = objectivefunction_ax.plot([], [], lw=3)

    def animate(trial_num):
        for i, b in enumerate(edge_dist_bars):
            b.set_height(edgedist[trial_num][i])
        edge_line_coll.set_segments(edges[trial_num])
        sites_line.set_data(sites[trial_num,:,0], sites[trial_num,:,1])
        perimeter_line.set_data(np.arange(trial_num), perimeters[:trial_num])
        objectivefunction_line.set_data(np.arange(trial_num), objectivefunctions[:trial_num])
        char_len_max_line.set_data(np.arange(trial_num), char_max_length[:trial_num])
        char_len_min_line.set_data(np.arange(trial_num), char_min_length[:trial_num])

    anim = matplotlib.animation.FuncAnimation(fig, animate, frames=nframes, interval=50, blit=False)
    anim.save('newest.mp4')

def myprint(string):
    if not args.silent:
        sys.stdout.write(string)
        sys.stdout.flush()

def arr(filename, dtype='float32'):
    return np.fromfile('output/' + filename, dtype=dtype);

def descent(args):
    # init vars
    myprint('rendering movie . . . ')
    nsites = len(open('input').readlines())
    linesegs_per_trial = 2*(3*nsites - 6)
    pts_per_lineseg = 2
    floats_per_pt = 2

    perimeter = arr('perimeter')
    sites = arr('sites')
    linesegs = arr('linesegs')
    char_max_length = arr('char_max_length')
    char_min_length = arr('char_min_length')
    objective_function = arr('objective_function')
    edgedist = arr('edgehist', dtype='int32')

    sites = sites.reshape((-1, nsites, 2))
    linesegs = linesegs.reshape((-1, linesegs_per_trial, pts_per_lineseg, floats_per_pt))
    edgedist = edgedist.reshape((-1, int(nsites * 1.4143)))

    # render
    if args.testing: 
        print(perimeter)
        print(sites)
        print(linesegs)
    else:
        render_animation(linesegs, sites, perimeter, objective_function, char_max_length, char_min_length, edgedist)
    log_time('\x1b[2K\r')


def generate_sites(num):
    f = open('input', 'w')
    for _ in range(num):
        rand = ((random()*1e6)//1) / 1e6
        rand2 = ((random()*1e6)//1) / 1e6
        f.write(str(rand) + '\t' + str(rand2) + '\n')
    f.close()

# ================================ ARGS ====================================== #
parser = argparse.ArgumentParser(description="render grain coarsening on voronoi diagrams")
parser.add_argument("-s", "--silent", action="store_true", default=False, help="don't show progress output")
parser.add_argument("-t", "--testing", action="store_true", default=False, help="output stuff for testing purposes")
parser.add_argument("-g", "--npoints", type=int, metavar="NUM", help="generate points and put them in the input file")

# ================================ MAIN ====================================== #
args = parser.parse_args()
if args.npoints:
    generate_sites(args.npoints)
else:
    descent(args)

