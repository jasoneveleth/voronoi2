import voronoi
import numpy as np
import matplotlib.collections
import matplotlib.animation
import matplotlib.pyplot as plt
import sys
from time import time
from random import random
import argparse

start = time()
np.set_printoptions(threshold=np.inf)

def log_time(string):
    global start
    myprint(string)
    myprint(f"elapsed: {(((time() - start)*1000)//1) / 1000} secs\n")
    start = time()

def render_animation(edges, sites, perimeters, objectivefunctions, char_max_length, char_min_length):
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

    char_len_ax.set_title('longest edge and shortest edge (characteristic length)')
    char_len_ax.set_xlim(0, nframes)
    char_len_ax.set_ylim(0, (4/3)*np.max(char_max_length))
    char_len_max_line, = char_len_ax.plot([], [], lw=3)
    char_len_min_line, = char_len_ax.plot([], [], lw=3)

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
        char_len_max_line.set_data(np.arange(trial_num), char_max_length[:trial_num])
        char_len_min_line.set_data(np.arange(trial_num), char_min_length[:trial_num])
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
    if not args.silent:
        sys.stdout.write(string)
        sys.stdout.flush()

def descent(args):
    # init vars
    nsites = len(open('input').readlines())
    linesegs_per_trial = 2*(3*nsites - 6)
    pts_per_lineseg = 2
    floats_per_pt = 2

    # allocate arrs
    linesegs = np.zeros((args.ntrials, linesegs_per_trial, pts_per_lineseg, floats_per_pt), 'float32') 
    sites = np.zeros((args.ntrials, nsites, 2), 'float32')
    perimeter = np.zeros((args.ntrials), 'float32')
    char_max_length = np.zeros((args.ntrials), 'float32')
    char_min_length = np.zeros((args.ntrials), 'float32')
    objectivefunctions = np.zeros((args.ntrials), 'float32')

    # descend
    myprint('descending . . .')
    voronoi.gradient_descent_func(args, linesegs, sites, perimeter, objectivefunctions, char_max_length, char_min_length)
    log_time('\rfinished descent\n')
    myprint('rendering . . .')

    # render
    if args.testing: 
        print(perimeter)
        print(sites)
        print(linesegs)
    else:
        render_animation(linesegs, sites, perimeter, objectivefunctions, char_max_length, char_min_length)
    log_time('\rfinished render\n')


def generate_sites(num):
    f = open('input', 'w')
    for _ in range(num):
        rand = ((random()*1e6)//1) / 1e6
        rand2 = ((random()*1e6)//1) / 1e6
        f.write(str(rand) + '\t' + str(rand2) + '\n')
    f.close()

objective_converter = { 
        'perimeter': 1, 
        'repulsion': 2 
        }
descent_method_converter = { 
        'constant_alpha': 0,
        'barziilai': 1,
        'conjugate': 2 
        }
gradient_method_converter = { 
        'finite_difference': 0 
        }
boundary_condition_converter = { 
        'bounce': 0, 'torus': 1 
        }

# ================================ MAIN ====================================== #
parser = argparse.ArgumentParser(description="compute and render grain coarsening on voronoi diagrams using gradient descent ", 
                                 epilog="this help output is low key confusing, so if you use ```grep -o '^.env/bin/python[^|]*' tests/main_test.sh``` you can see the tests (and example calls)",
                                 formatter_class=argparse.ArgumentDefaultsHelpFormatter)
group = parser.add_mutually_exclusive_group()

parser.add_argument("-s", "--silent", action="store_true", default=False, help="don't show progress output")
parser.add_argument("-t", "--testing", action="store_true", default=False, help="output stuff for testing purposes")
parser.add_argument("--file", default='input', help="the file that the site point coords are read from")
parser.add_argument("--objective", choices=['repulsion', 'perimeter'], nargs='+', default=['perimeter'], help="the objective function you are minimizing, you can supply multiple arguments to it if you care about multiple things (and you can't just specify repulsion)")
parser.add_argument("--descent", choices=['constant_alpha', 'barziilai'], default='constant_alpha', help="the gradient descent method to use")
parser.add_argument("--gradient", choices=['finite_difference'], default='finite_difference', help="the gradient method to use")
parser.add_argument("--boundary", choices=['torus', 'bounce'], default='bounce', help="how the points wrap when they are on the margin")
parser.add_argument("--alpha", type=float, default=3e-3, help="the step size if the descent method is constants_alpha")
parser.add_argument("--repel_coeff", type=float, default=1e-4, help="the strength of the repulsion of points")
parser.add_argument("--jiggle", type=float, default=1e-4, help="the amount jiggle for each point's coords when doing finite difference gradient")

group.add_argument("-n", "--ntrials", type=int, metavar="NUM", default=50, help="perform gradient descent for that many trials")
group.add_argument("-g", "--npoints", type=int, metavar="NUM", help="generate points and put them in the input file")

args = parser.parse_args()
args.objective = sum([objective_converter[i] for i in args.objective])
args.descent = descent_method_converter[args.descent]
args.gradient = gradient_method_converter[args.gradient]
args.boundary = boundary_condition_converter[args.boundary]

if args.npoints:
    generate_sites(args.npoints)
else:
    descent(args)

