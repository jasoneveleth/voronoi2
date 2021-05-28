import voronoi
import numpy as np
import matplotlib.collections
import matplotlib.pyplot as plt

def plotAnimation(collection, fileNum=''):
    numFrames = len(collection)
    fig = plt.figure()
    fig.subplots_adjust(hspace=0.4, wspace=0.4)
    ax1 = fig.add_subplot(2, 1, 1)
    ax2 = fig.add_subplot(2, 1, 2, aspect='equal')
    perimeters = []
    for e in collection:
        perimeters.append(e[1])
    ax1.set_xlim(0, numFrames)
    ax1.set_ylim(0, 4*max(perimeters)/3) # EWWWWW hard coded
    ax2.set_xlim(0, 1)
    ax2.set_ylim(0, 1)
    ax1.set_title('gamma function (perimeter)')
    ax2.set_title('voronoi diagram')

    gammaLine, = ax1.plot([], [], lw=3) # the comma unpacks the tuple
    edges = matplotlib.collections.LineCollection(())
    sites, = ax2.plot([], [], 'ro')
    ax2.add_collection(edges)
    gamma = []

    def animate(i):
        tempE, tempS = collection[i][0]
        edges.set_segments(tempE)
        tempS = np.array(list(tempS))
        # tempS = np.fromiter(tempS, 'float,float').reshape((len(tempS),2))
        sites.set_data(tempS[:,0], tempS[:,1])

        gamma.append(collection[i][1])
        x = np.arange(len(gamma))
        y = np.array(gamma)
        gammaLine.set_data(x, y)
        return gammaLine,edges,sites,

    anim = FuncAnimation(fig, animate, frames=numFrames, interval=20, blit=True)
    anim.save(f'visuals/newest.gif', writer='imagemagick')

def plot(edges, sites):
    plt.axis([0, 1, 0, 1])
    sites = np.array(sites)
    edges = matplotlib.collections.LineCollection(edges)
    plt.plot(sites[:,0], sites[:,1], 'ro')
    plt.gca().add_collection(edges)
    plt.show()

edges = np.zeros((30), 'float32')
voronoi.simple_diagram_func(edges)

# sites = np.array([
#     (0.342, 0.23423),
#     (0.85674, 0.75645),
#     (0.43525, 0.987867)
#     ])
# three edges each with two points of 2 coordinates
# edges = np.zeros((3, 2, 2), 'float');
# edges[0][0][0] = 0.23423
# edges[0][0][1] = 0.342
# edges[0][1][0] = 0.43525
# edges[0][1][1] = 0.75645
# edges[1][0][0] = 0.85674
# edges[1][0][1] = 0.987867
# edges[1][1][0] = 0.3248
# edges[1][1][1] = 0.6478
# edges[2][0][0] = 0.18763
# edges[2][0][1] = 0.16378
# edges[2][1][0] = 0.9832
# edges[2][1][1] = 0.4783
# line_collection = matplotlib.collections.LineCollection(edges)
# plt.axis([0, 1, 0, 1])
# plt.gca().add_collection(line_collection)
# plt.plot(sites[:,0], sites[:,1], 'ro')
# plt.show()

