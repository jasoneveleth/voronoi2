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

    fig = plt.figure()
    fig.set_figwidth(8)
    fig.set_figheight(8)
    # fig.subplots_adjust(hspace=0.4, wspace=0.4)

    ax1 = fig.add_subplot(2, 1, 1)
    ax1.set_xlim(0, nframes)
    ax1.set_ylim(0, 4*np.max(perimeters)/3)
    ax1.set_title('perimeter (objective function)')
    perimeter_line, = ax1.plot([], [], lw=3) # the comma unpacks the tuple

    ax2 = fig.add_subplot(2, 1, 2, aspect='equal')
    ax2.set_xlim(0, 1)
    ax2.set_ylim(0, 1)
    ax2.set_title('voronoi diagram')
    sites_line, = ax2.plot([], [], 'ro')

    edge_line_coll = matplotlib.collections.LineCollection(())
    ax2.add_collection(edge_line_coll)

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

