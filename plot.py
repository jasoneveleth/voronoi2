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
    perimeter_ax = fig.add_subplot(4, 2, 1)
    char_len_ax = fig.add_subplot(4, 2, 2)
    earth_mover_ax = fig.add_subplot(4, 2, 3)
    edge_dist_ax = fig.add_subplot(4, 2, 4)
    objectivefunction_ax = fig.add_subplot(4, 2, 5)
    diagram_ax = fig.add_subplot(4, 2, (6, 8), aspect='equal')

    perimeter_ax.set_title('perimeter')
    perimeter_ax.set_xlim(0, nframes)
    perimeter_ax.set_ylim(0, (4/3)*np.max(perimeters))
    perimeter_line, = perimeter_ax.plot([], [], lw=3) # the comma unpacks the tuple

    diagram_ax.set_title('voronoi diagram')
    diagram_ax.set_xlim(0, 1)
    diagram_ax.set_ylim(0, 1)
    edge_line_coll = matplotlib.collections.LineCollection(())
    diagram_ax.add_collection(edge_line_coll)
    sites_line, = diagram_ax.plot([], [], 'ro')

    char_len_ax.set_title('longest edge and shortest edge (characteristic length)')
    char_len_ax.set_xlim(0, nframes)
    char_len_ax.set_ylim(0, (4/3)*np.max(char_max_length))
    char_len_max_line, = char_len_ax.plot([], [], lw=3)
    char_len_min_line, = char_len_ax.plot([], [], lw=3)

    earth_mover_ax.set_title('earth mover distance')
    earth_mover_ax.set_xlim(0, nframes)

    edge_dist_ax.set_title('edge distribution')
    edge_dist_ax.set_xlim(0, 1.4143)
    edge_dist_ax.set_ylim(0, np.max(edgedist) * (4/3))
    nbars = int(sites.shape[1] * 1.4143)
    x = np.linspace(0, 1.4143, num=nbars, endpoint=False)
    edge_dist_bars = edge_dist_ax.bar(x, edgedist[0], width=(1/sites.shape[1]), align='edge')

    objectivefunction_ax.set_title('objective function')
    objectivefunction_ax.set_xlim(0, nframes)
    objectivefunction_ax.set_ylim(0, (4/3)*np.max(objectivefunctions))
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

def descent(args):
    # init vars
    myprint('rendering movie . . . ')
    nsites = len(open('input').readlines())
    linesegs_per_trial = 2*(3*nsites - 6)
    pts_per_lineseg = 2
    floats_per_pt = 2

    perimeter = np.fromfile('output/perimeter', dtype='float32')
    sites = np.fromfile('output/sites', dtype='float32')
    linesegs = np.fromfile('output/linesegs', dtype='float32')
    char_max_length = np.fromfile('output/char_max_length', dtype='float32')
    char_min_length = np.fromfile('output/char_min_length', dtype='float32')
    objective_function = np.fromfile('output/objective_function', dtype='float32')
    edgedist = np.fromfile('output/edgehist', dtype='int32')

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

