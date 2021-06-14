# Voronoi Diagrams

## Install

```bash
$ git clone https://github.com/jasoneveleth/voronoi2
$ cd voronoi2
$ python3 -m venv .env
$ . .env/bin/activate
$ python -m pip install Cython matplotlib numpy
$ make run
```

This will make 'newest.gif' in the current directory. Look at it with `open
newest.gif`

To run it with your own points, add them to 'input' file. And rerun the main.py
file with the correct options:

```bash
$ vi input
$ python main.py -n 50
```

Check options with `$ python main.py -h`

## Notes

- a '+' at the end of git messages means this commit could be split into
  multiple parts

## Updates

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

Turns out I knew what I was doing when I didn't go with the array implementaion 
of a binary tree. That works great and is super elegant until you are trying to 
rearrange the tree in a nontrivial way - like skipping over a node by 
reassigning pointers. To do that it'd be like O(n) or something idk. Anyway, I 
reimplemented everything and now it works (at least for 3 points)!

So, I had a lot of memory issues, luckily valgrind really saved the day here. I 
didn't initialize a lot of fields in structs to NULL, like bnodes and hnodes. 
That caused undefined jumping behavior if that memory wasn't right. I also 
forgot to reset the children of the parent's '-\>parent' which was leading to a 
weird tree.

## Debug

- I had the wrong criteria for being an internal node, I said both children 
need to be null, when what I meant was that one of them needs to be null but 
not necessarily. Also didn't null check.

- I had the wrong sign for calculating if two vectors formed a right turn, I
  was calculating if they took a left turn

- was always returning biggest then smallest, when I thought it was smallest
  then biggest

- I didn't initialize a lot of fields in structs to NULL, like bnodes and hnodes. 
  That caused undefined jumping behavior if that memory wasn't right. 

- I also forgot to reset the children of the parent's '-\>parent' which was 
  leading to a weird tree.

- didn't initialize pointer to struct

- didn't go from half edges to edges by multiplying by 2

## Possible speed optimizations (test to make sure it is an issue)

- multithread each trial
- rebalance tree
- inline edges array
