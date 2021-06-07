import voronoi
import numpy as np
import matplotlib.collections
import matplotlib.animation
import matplotlib.pyplot as plt
import sys
from time import time
from random import random

# commandline flags
suppress_output = 0
testing_mode = 0 
global_ntrials = 50

start = time()
np.set_printoptions(threshold=np.inf)

def log_time(string):
    global start
    myprint(string)
    myprint(f"elapsed: {(((time() - start)*1000)//1) / 1000} secs\n")
    start = time()

def render_animation(edges, sites, perimeters, objectivefunctions):
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
    qr_rates_ax = fig.add_subplot(4, 2, 4)
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

    char_len_ax.set_title('longest edge/shortest edge (characteristic length)')
    char_len_ax.set_xlim(0, nframes)

    earth_mover_ax.set_title('earth mover distance')
    earth_mover_ax.set_xlim(0, nframes)

    qr_rates_ax.set_title('q and r rates')
    qr_rates_ax.set_xlim(0, nframes)

    objectivefunction_ax.set_title('objective function')
    objectivefunction_ax.set_xlim(0, nframes)
    objectivefunction_ax.set_ylim(0, (4/3)*np.max(objectivefunctions))
    objectivefunction_line, = objectivefunction_ax.plot([], [], lw=3)

    def animate(trial_num):
        edge_line_coll.set_segments(edges[trial_num])
        sites_line.set_data(sites[trial_num,:,0], sites[trial_num,:,1])
        perimeter_line.set_data(np.arange(trial_num), perimeters[:trial_num])
        objectivefunction_line.set_data(np.arange(trial_num), objectivefunctions[:trial_num])
        return perimeter_line,edge_line_coll,sites_line,

    anim = matplotlib.animation.FuncAnimation(fig, animate, frames=nframes, interval=20, blit=True)
    anim.save('newest.gif') # writer='ffmpeg'

def plot_diagram(edges, sites):
    """ edges - numpy arr (n, 2, 2)
        sites - numpy arr (m, 2)"""
    plt.axis([0, 1, 0, 1])
    line_coll = matplotlib.collections.LineCollection(edges)
    plt.plot(sites[:,0], sites[:,1], 'ro')
    plt.gca().add_collection(line_coll)
    plt.show()

def default():
    edges = np.zeros((1000, 2, 2), 'float32') # edges each with two points (2 coordinates)
    sites = np.zeros((100, 2), 'float32') # 31 sites of 1 point (2 coordinates)
    voronoi.simple_diagram_func(edges, sites)
    plot_diagram(edges, sites)

def myprint(string):
    if not suppress_output:
        sys.stdout.write(string)
        sys.stdout.flush()

def descent(ntrials, jiggle):
    # init vars
    nsites = len(open('input').readlines())
    linesegs_per_trial = 2*(3*nsites - 6)
    pts_per_lineseg = 2
    floats_per_pt = 2

    # allocate arrs
    linesegs = np.zeros((ntrials, linesegs_per_trial, pts_per_lineseg, floats_per_pt), 'float32') 
    sites = np.zeros((ntrials, nsites, 2), 'float32')
    perimeter = np.zeros((ntrials), 'float32')
    objectivefunctions = np.zeros((ntrials), 'float32')

    # descend
    myprint('descending . . .')
    voronoi.gradient_descent_func(linesegs, sites, perimeter, objectivefunctions, jiggle)
    log_time('\rfinished descent\n')
    myprint('rendering . . .')

    # render
    if testing_mode: 
        print(perimeter)
        print(sites)
        print(linesegs)
    else:
        render_animation(linesegs, sites, perimeter, objectivefunctions)
    log_time('\rfinished render\n')


def generate_sites(num):
    f = open('input', 'w')
    for _ in range(num):
        rand = ((random()*1e6)//1) / 1e6
        rand2 = ((random()*1e6)//1) / 1e6
        f.write(str(rand) + '\t' + str(rand2) + '\n')
    f.close()

# ================================ MAIN ====================================== #

if '-h' in sys.argv:
    print(f'usage: python {sys.argv[0]} [-n num] [-s] [-t] [-g num_points]')
    print('\t-n\tgradient descent on "num" number of trials')
    print('\t-s\tsilent mode')
    print('\t-t\toutput stuff for testing')
    print('\t-g\tgenerate "num" number of points, NOTE overrides -n')
    exit()
if '-s' in sys.argv:
    suppress_output = 1
if '-t' in sys.argv:
    testing_mode = 1
if '-n' in sys.argv:
    global_ntrials = int(sys.argv[sys.argv.index('-n')+1])

if '-g' in sys.argv:
    generate_sites(int(sys.argv[sys.argv.index('-g')+1]))
elif '-b' in sys.argv:
    descent(global_ntrials, 1e-4)
else:
    descent(global_ntrials, 1e-4)

