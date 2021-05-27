### Notes

when I'm trying to make a pointer to an element in the heap, rather than make a 
pointer to that place in memory (the place in the array), just take the pointer 
to the value - if you need the place in memory, just use the index as an offset 
from the heap's arr pointer.

Need `clang-format` in PATH

[code](https://www3.cs.stonybrook.edu/~algorith/implement/fortune/distrib/)
[paper](https://www.math.cmu.edu/users/slepcev/voronoi_curvature_flow.pdf)
[valgrind](https://www.cprogramming.com/debugging/valgrind.html)
[embedding python in c](https://www.codeproject.com/Articles/11805/Embedding-Python-in-C-C-Part-I)

# Updates

So I realized that I invented a lot of complexity when I wrote this the last 
time. After looking at [this 
implementation](https://www.cs.hmc.edu/~mbrubeck/voronoi.html) that the way to 
extend the diagram is to measure the longest diagonal of the bounding box, and 
subtract it from the bottom of the diagram. The only way for part of parabola to 
be "inside the square", it would need to have that part of the square (the part 
that is below [below meaning more negative y] the parabola) to be farther from 
the point than it is from the sweep line. We make that impossible by moving the  
status line far down enough that the longest straight line is too short. Thus, 
the formula is `ymin - sqrt((xmax - xmin)^2 + (ymax - ymin)^2)`.

# Debug

- I had the wrong criteria for being an internal node, I said both children 
need to be null, when what I meant was that one of them needs to be null but 
not necessarily. Also didn't null check.

- I had the wrong sign for calculating if two vectors formed a right turn, I
  was calculating if they took a left turn

- Didn't update the index of the nodes in bintree (MAYBE)
