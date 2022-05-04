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

def arr(filename, directory='output/', dtype='float32'):
    return np.fromfile(directory + filename, dtype=dtype);

# rename 'output/' to 'dc/' 'ds/' and 'db/' for constant, steepest, and barzilai repectively
def multi(speed=0):
    nsites = len(open('input').readlines())
    linesegs_shape = (-1, 2*(3*nsites - 6), 2, 2)

    perimeters = [arr('perimeter', 'dc/'), arr('perimeter', 'db/'), arr('perimeter', 'ds/')]
    sites = [arr('sites', 'dc/'), arr('sites', 'db/'), arr('sites', 'ds/')]
    linesegs = [arr('linesegs', 'dc/'), arr('linesegs', 'db/'), arr('linesegs', 'ds/')]
    char_max_length = [arr('char_max_length', 'dc/'), arr('char_max_length', 'db/'), arr('char_max_length', 'ds/')]
    char_min_length = [arr('char_min_length', 'dc/'), arr('char_min_length', 'db/'), arr('char_min_length', 'ds/')]
    objfunc = [arr('objective_function', 'dc/'), arr('objective_function', 'db/'), arr('objective_function', 'ds/')]
    edgedist = [arr('edgehist', 'dc/'), arr('edgehist', 'db/'), arr('edgehist', 'ds/')]
    earthmover = [arr('earthmover', 'dc/'), arr('earthmover', 'db/'), arr('earthmover', 'ds/')]
    alpha = [arr('alpha', 'dc/'), arr('alpha', 'db/'), arr('alpha', 'ds/')]

    for i in range(3):
        sites[i] = sites[i].reshape((-1, nsites, 2))
        linesegs[i] = linesegs[i].reshape(linesegs_shape)
        edgedist[i] = edgedist[i].reshape((-1, int(nsites * 1.4143)))

    graph_len = sites[0].shape[0]
    nframes = graph_len
    if speed != 0:
        nframes = min(int(math.fsum(alpha[0]) / speed), int(math.fsum(alpha[1]) / speed), int(math.fsum(alpha[2]) / speed))

    fig = plt.figure()
    fig.set_size_inches(16, 10)
    fig.subplots_adjust(left=0.05, bottom=0.05, right=0.95, top=0.95, wspace=0.3, hspace=0.3)

    diagram_axs = (fig.add_subplot(231, aspect='equal'), fig.add_subplot(232, aspect='equal'), fig.add_subplot(233, aspect='equal'))
    setup_ax(diagram_axs[0], 'steepest w/o linesearch', (0, 1), (0, 1))
    setup_ax(diagram_axs[1], 'barzilai borwein', (0, 1), (0, 1))
    setup_ax(diagram_axs[2], 'steepest w/ linsearch', (0, 1), (0, 1))

    edgedist_ax = (fig.add_subplot(437), fig.add_subplot(438), fig.add_subplot(439))
    edge_dist_bars = []
    for i in range(3):
        view = 5/np.sqrt(nsites)
        setup_ax(edgedist_ax[i], 'edge distribution', (-view/50, view), (0, np.max(edgedist) * (4/3)))
        # HARDCODE 1.4143
        nbars = int(sites[i].shape[1] * 1.4143)
        x = np.linspace(0, 1.4143, num=nbars, endpoint=False)
        edge_dist_bars.append(edgedist_ax[i].bar(x, edgedist[i][0], width=(1/sites[i].shape[1]), align='edge'))

    objfunc_ax = fig.add_subplot(4, 4, 13)
    setup_ax(objfunc_ax, 'f(X)', (1, graph_len), (0, (4/3)*np.max(objfunc)))
    objfunc_line = []
    objfunc_line.append(objfunc_ax.plot([], [], lw=3, label="w/o linesearch")[0])
    objfunc_line.append(objfunc_ax.plot([], [], lw=3, label="Barzilai")[0])
    objfunc_line.append(objfunc_ax.plot([], [], lw=3, label="w linesearch")[0])
    objfunc_ax.legend()

    alpha_ax = fig.add_subplot(4, 4, 14)
    setup_ax(alpha_ax, "stepsize", (1, graph_len), (0, 0.1))
    alpha_line = []
    alpha_line.append(alpha_ax.plot([], [], lw=3, label="w/o linesearch")[0])
    alpha_line.append(alpha_ax.plot([], [], lw=3, label="Barzilai")[0])
    alpha_line.append(alpha_ax.plot([], [], lw=3, label="w linesearch")[0])
    alpha_ax.legend()

    earthmover_ax = fig.add_subplot(4, 4, 15)
    setup_ax(earthmover_ax, "earthmover", (1, graph_len), (0, (4/3)*np.max(earthmover)))
    earthmoverline = []
    earthmoverline.append(earthmover_ax.plot([], [], lw=3, label="w/o linesearch")[0])
    earthmoverline.append(earthmover_ax.plot([], [], lw=3, label="Barzilai")[0])
    earthmoverline.append(earthmover_ax.plot([], [], lw=3, label="w linesearch")[0])
    earthmover_ax.legend()

    perimeter_ax = fig.add_subplot(4, 4, 16)
    setup_ax(perimeter_ax, "perimeter", (1, graph_len), (0, (4/3)*np.max(perimeters)))
    perimeter_line = []
    perimeter_line.append(perimeter_ax.plot([], [], lw=3, label="w/o linesearch")[0])
    perimeter_line.append(perimeter_ax.plot([], [], lw=3, label="Barzilai")[0])
    perimeter_line.append(perimeter_ax.plot([], [], lw=3, label="w linesearch")[0])
    perimeter_ax.legend()

    edge_line_coll = (matplotlib.collections.LineCollection(()), matplotlib.collections.LineCollection(()), matplotlib.collections.LineCollection(()))
    sites_line = []
    for i in range(3):
        diagram_axs[i].add_collection(edge_line_coll[i])
        line, = diagram_axs[i].plot([], [], 'ro', ms=5)
        sites_line.append(line)

    # dumb but needed
    def init():
        pass

    def animate(frame_num):
        trial_num = [frame_num, frame_num, frame_num]
        if speed != 0:
            for j in range(3):
                counter = 0
                for i,_ in enumerate(alpha[j]):
                    if (math.fsum(alpha[j][:i+1]) > speed * frame_num):
                        break
                    counter += 1
                trial_num[j] = counter

        print(trial_num)
        for i in range(3):
            end = trial_num[i] + 1 # +1 because end is not included in range
            edge_line_coll[i].set_segments(linesegs[i][trial_num[i]])
            sites_line[i].set_data(sites[i][trial_num[i],:,0], sites[i][trial_num[i],:,1])
            for j, b in enumerate(edge_dist_bars[i]):
                b.set_height(edgedist[i][trial_num[i]][j])
            objfunc_line[i].set_data(np.arange(1, end+1), objfunc[i][:end])
            alpha_line[i].set_data(np.arange(1, end+1), alpha[i][:end])
            earthmoverline[i].set_data(np.arange(1, end+1), earthmover[i][:end])
            perimeter_line[i].set_data(np.arange(1, end+1), perimeters[i][:end])
        myprint(f'\rrender trial: {frame_num} ')

    anim = matplotlib.animation.FuncAnimation(fig, animate, init_func=init, frames=nframes, interval=50, blit=False)
    anim.save('newest.mp4')
    log_time('\x1b[2K\r')

def just_diagram(speed=0):
    nsites = len(open('input').readlines())
    linesegs_shape = (-1, 2*(3*nsites - 6), 2, 2)

    sites = arr('sites')
    linesegs = arr('linesegs')
    edgedist = arr('edgehist')
    alpha = arr('alpha')

    sites = sites.reshape((-1, nsites, 2))
    linesegs = linesegs.reshape(linesegs_shape)
    edgedist = edgedist.reshape((-1, int(nsites * 1.4143)))

    graph_len = sites.shape[0]
    nframes = graph_len
    if speed != 0:
        nframes = int(math.fsum(alpha) / speed)

    fig = plt.figure()
    fig.set_size_inches(10, 10)
    fig.subplots_adjust(left=0.05, bottom=0.05, right=0.95, top=0.95, wspace=0.3, hspace=0.3)

    diagram_ax = fig.add_subplot(111, aspect='equal')

    setup_ax(diagram_ax, 'voronoi diagram', (0, 1), (0, 1))
    edge_line_coll = matplotlib.collections.LineCollection(())
    diagram_ax.add_collection(edge_line_coll)
    sites_line, = diagram_ax.plot([], [], 'ro', ms=5)

    # dumb but needed
    def init():
        pass

    def animate(frame_num):
        trial_num = frame_num
        if speed != 0:
            counter = 0
            for i,_ in enumerate(alpha):
                if (math.fsum(alpha[:i+1]) > speed * frame_num):
                    break
                counter += 1
            myprint(f'sum: {math.fsum(alpha[:counter+1])}, goal: {speed * frame_num} ')
            trial_num = counter

        edge_line_coll.set_segments(linesegs[trial_num])
        sites_line.set_data(sites[trial_num,:,0], sites[trial_num,:,1])
        myprint(f'\rrender trial: {trial_num} ')

    anim = matplotlib.animation.FuncAnimation(fig, animate, init_func=init, frames=nframes, interval=50, blit=False)
    anim.save('newest.mp4')
    log_time('\x1b[2K\r')


def render(speed=0):
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

    graph_len = sites.shape[0]
    nframes = graph_len
    if speed != 0:
        nframes = int(math.fsum(alpha) / speed)

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
        trial_num = frame_num
        if speed != 0:
            counter = 0
            for i,_ in enumerate(alpha):
                if (math.fsum(alpha[:i+1]) > speed * frame_num):
                    break
                counter += 1
            myprint(f'sum: {math.fsum(alpha[:counter+1])}, goal: {speed * frame_num} ')
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
    # multi(speed=3e-3)
    # just_diagram()
    # render(speed=3e-3)

