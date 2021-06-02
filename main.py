import voronoi
import numpy as np
import matplotlib.collections
import matplotlib.animation
import matplotlib.pyplot as plt

def plot_animation(edges, sites, perimeters):
    """ perimeters : numpy arr (n, 1)
        sites : numpy arr (n, m, 2)
        edges : numpy arr (n, 3*m-6, 2, 2)
    """
    nframes = sites.shape[0]

    fig = plt.figure()
    fig.subplots_adjust(hspace=0.4, wspace=0.4)

    ax1 = fig.add_subplot(2, 1, 1)
    ax1.set_xlim(0, nframes)
    ax1.set_ylim(0, 4*np.max(perimeters)/3)
    ax1.set_title('gamma function (perimeter)')
    perimeter_line, = ax1.plot([], [], lw=3) # the comma unpacks the tuple

    ax2 = fig.add_subplot(2, 1, 2, aspect='equal')
    ax2.set_xlim(0, 1)
    ax2.set_ylim(0, 1)
    ax2.set_title('voronoi diagram')
    sites_line, = ax2.plot([], [], 'ro')

    edge_line_coll = matplotlib.collections.LineCollection(())
    ax2.add_collection(edge_line_coll)

    def animate(i):
        edge_line_coll.set_segments(edges[i])
        sites_line.set_data(sites[i,0], sites[i,1])
        gamma_line.set_data(np.arange(i), gamma[:i])
        return gamma_line,edges,sites,

    anim = matplotlib.animation.FuncAnimation(fig, animate, frames=nframes, interval=20, blit=True)
    # try write='imagemagick' writer='ffmpeg' and nothing to see the fastest 
    # animation.writers.list()
    # https://stackoverflow.com/questions/4092927/generating-movie-from-python-without-saving-individual-frames-to-files
    anim.save('newest.gif', writer='imagemagick')

def plot_diagram(edges, sites):
    """ edges - numpy arr (n, 2, 2)
        sites - numpy arr (m, 2)"""
    plt.axis([0, 1, 0, 1])
    line_coll = matplotlib.collections.LineCollection(edges)
    plt.plot(sites[:,0], sites[:,1], 'ro')
    plt.gca().add_collection(line_coll)
    plt.show()

# edges = np.zeros((1000, 2, 2), 'float32') # edges each with two points (2 coordinates)
# sites = np.zeros((100, 2), 'float32') # 31 sites of 1 point (2 coordinates)
# voronoi.simple_diagram_func(edges, sites)
# plot_diagram(edges, sites)

ntrials = 3
nsites = 100
linesegs = np.zeros((ntrials, 3*nsites - 6, 2, 2), 'float32') # trials, linesegs/trial, pts/seg, floats/pt
sites = np.zeros((ntrials, nsites, 2), 'float32') # 31 sites of 1 point (2 coordinates)
perimeter = np.zeros((ntrials), 'float32')
voronoi.gradient_descent_func(linesegs, sites, perimeter, 1e-5)
# np.set_printoptions(threshold=np.inf)
# print(perimeter)
# print(sites)
# print(linesegs)
plot_animation(linesegs, sites, perimeter)

