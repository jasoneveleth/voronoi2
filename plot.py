import numpy as np
import math
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

def test():
    nsites = len(open('input').readlines())
    linesegs_shape = (-1, 2*(3*nsites - 6), 2, 2)
    print(arr('perimeter'))
    print(arr('sites').reshape((-1, nsites, 2)))
    print(arr('linesegs').reshape(linesegs_shape))

def setup_ax(ax, title, xlim, ylim):
    ax.set_title(title)
    ax.set_xlim(xlim[0], xlim[1])
    ax.set_ylim(ylim[0], ylim[1])

def arr(filename, dtype='float32'):
    return np.fromfile('output/' + filename, dtype=dtype);

def render():
    nsites = len(open('input').readlines())
    linesegs_shape = (-1, 2*(3*nsites - 6), 2, 2)

    perimeters = arr('perimeter')
    sites = arr('sites')
    linesegs = arr('linesegs')
    char_max_length = arr('char_max_length')
    char_min_length = arr('char_min_length')
    objfunc = arr('objective_function')
    edgedist = arr('edgehist')
    earthmover = arr('earthmover')
    alpha = arr('alpha')

    sites = sites.reshape((-1, nsites, 2))
    linesegs = linesegs.reshape(linesegs_shape)
    edgedist = edgedist.reshape((-1, int(nsites * 1.4143)))

    nframes = int(math.fsum(alpha) / 3e-3)
    graph_len = sites.shape[0]

    fig, axs = plt.subplots(nrows=4, ncols=2)
    fig.subplots_adjust(left=0.03, bottom=0.03, right=0.97, top=0.97, wspace=0.3, hspace=0.3)
    fig.set_size_inches(16, 10)

    axs[2, 1].remove()
    axs[3, 1].remove()
    diagram_ax = fig.add_subplot(4, 2, (6, 8), aspect='equal')

    setup_ax(axs[0, 0], 'perimeter', (1, graph_len), (0, (4/3)*np.max(perimeters)))
    perimeter_line, = axs[0, 0].plot([], [], lw=3) # the comma unpacks the tuple

    setup_ax(diagram_ax, 'voronoi diagram', (0, 1), (0, 1))
    edge_line_coll = matplotlib.collections.LineCollection(())
    diagram_ax.add_collection(edge_line_coll)
    sites_line, = diagram_ax.plot([], [], 'ro', ms=5)

    setup_ax(axs[0, 1], 'longest edge and shortest edge (characteristic length)', (1, graph_len), (0, (4/3)*np.max(char_max_length)))
    char_len_max_line, = axs[0, 1].plot([], [], lw=3)
    char_len_min_line, = axs[0, 1].plot([], [], lw=3)

    setup_ax(axs[1, 0], 'earth mover distance', (1, graph_len), (0, (4/3) * np.max(earthmover)))
    earthmover_line, = axs[1,0].plot([], [], lw=3)

    view = 5/np.sqrt(nsites)
    setup_ax(axs[1, 1], 'edge distribution', (-view/50, view), (0, np.max(edgedist) * (4/3)))
    # HARDCODE 1.4143
    nbars = int(sites.shape[1] * 1.4143)
    x = np.linspace(0, 1.4143, num=nbars, endpoint=False)
    edge_dist_bars = axs[1,1].bar(x, edgedist[0], width=(1/sites.shape[1]), align='edge')

    setup_ax(axs[2, 0], 'objective function', (1, graph_len), (0, (4/3)*np.max(objfunc)))
    objectivefunction_line, = axs[2,0].plot([], [], lw=3)

    setup_ax(axs[3,0], 'alpha', (1, graph_len), (0, 1))
    alpha_line, = axs[3,0].plot([], [], lw=3)

    # dumb but needed
    def init():
        pass

    def animate(frame_num):
        counter = 0
        for i,_ in enumerate(alpha):
            if (math.fsum(alpha[:i+1]) > 3e-3 * frame_num):
                break
            counter += 1

        trial_num = counter
        end = trial_num + 1 # +1 because end is not included in range
        for i, b in enumerate(edge_dist_bars):
            b.set_height(edgedist[trial_num][i])
        edge_line_coll.set_segments(linesegs[trial_num])
        sites_line.set_data(sites[trial_num,:,0], sites[trial_num,:,1])
        perimeter_line.set_data(np.arange(1, end+1), perimeters[:end])
        objectivefunction_line.set_data(np.arange(1, end+1), objfunc[:end])
        char_len_max_line.set_data(np.arange(1, end+1), char_max_length[:end])
        char_len_min_line.set_data(np.arange(1, end+1), char_min_length[:end])
        earthmover_line.set_data(np.arange(1, end+1), earthmover[:end])
        alpha_line.set_data(np.arange(1, end+1), alpha[:end])
        myprint(f'\rrender trial: {trial_num} ')

    anim = matplotlib.animation.FuncAnimation(fig, animate, init_func=init, frames=nframes, interval=50, blit=False)
    anim.save('newest.mp4')
    log_time('\x1b[2K\r')

def myprint(string):
    if not args.silent:
        sys.stdout.write(string)
        sys.stdout.flush()

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
elif args.testing:
    test()
else:
    render()

