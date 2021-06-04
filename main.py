import voronoi
import numpy as np
import matplotlib.collections
import matplotlib.animation
import matplotlib.pyplot as plt
from sys import argv
from time import time
from random import random

# commandline flags
suppress_output = 0
output_tests = 0 
global_ntrials = 50

def render_animation(edges, sites, perimeters):
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
    diagram_ax = fig.add_subplot(4, 2, (5, 8), aspect='equal')

    perimeter_ax.set_title('perimeter (objective function)')
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

    def animate(trial_num):
        edge_line_coll.set_segments(edges[trial_num])
        sites_line.set_data(sites[trial_num,:,0], sites[trial_num,:,1])
        perimeter_line.set_data(np.arange(trial_num), perimeters[:trial_num])
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

def descent(ntrials):
    start = time()
    nsites = len(open('input').readlines())
                        # trials, linesegs(== halfedges)/trial, pts/lineseg, floats/pt
    linesegs = np.zeros((ntrials, 2*(3*nsites - 6), 2, 2), 'float32') 
    sites = np.zeros((ntrials, nsites, 2), 'float32')
    perimeter = np.zeros((ntrials), 'float32')
    if not suppress_output: print("\ndescending . . .")
    voronoi.gradient_descent_func(linesegs, sites, perimeter, 1e-4)
    np.set_printoptions(threshold=np.inf)
    if output_tests: print(perimeter)
    if output_tests: print(sites)
    if output_tests: print(linesegs)
    if not suppress_output: print("plotting . . .")
    render_animation(linesegs, sites, perimeter)
    if not suppress_output: print(f"elapsed: {(((time() - start)*1000)//1) / 1000} secs")


def generate_sites(num):
    f = open('input', 'w')
    for _ in range(num):
        rand = ((random()*1e6)//1) / 1e6
        rand2 = ((random()*1e6)//1) / 1e6
        f.write(str(rand) + '\t' + str(rand2) + '\n')
    f.close()

# ================================ MAIN ====================================== #

if '-s' in argv:
    suppress_output = 1
if '-t' in argv:
    output_tests = 1
if '-n' in argv:
    global_ntrials = int(argv[argv.index('-n')+1])

if '-g' in argv:
    generate_sites(int(argv[argv.index('-g')+1]))
else:
    descent(global_ntrials)

