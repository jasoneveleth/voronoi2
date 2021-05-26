FLAGS = -std=c99 -Werror -Weverything -Wno-poison-system-directories
FLAGS += -Ofast
# FLAGS += -g -O0 -Wno-unused-parameter -Wno-unused-variable
FLAGS += -DFLOAT
C = heap.c bintree.h heap.h test.c bintree.c

.PHONY: all clean test debug format

all: format short_test voronoi

format:
	clang-format -i -style=file $C

bintree.o: bintree.c bintree.h
	clang $(FLAGS) -c bintree.c

heap.o: heap.c heap.h
	clang $(FLAGS) -c heap.c

short_test: test.c heap.o bintree.o
	clang $(FLAGS) heap.o bintree.o test.c -o short_test

test: format short_test
	./short_test

voronoi: format heap.o bintree.o
	clang $(FLAGS) heap.o bintree.o voronoi.c -o voronoi

# debug

debug_heap.o: heap.c heap.h
	clang $(FLAGS) -DDEBUG -c heap.c -o debug_heap.o

debug_bintree.o: bintree.c bintree.h
	clang $(FLAGS) -DDEBUG -c bintree.c -o debug_bintree.o

debug_test: test.c debug_heap.o
	clang  $(FLAGS) -DDEBUG debug_heap.o test.c -o debug_test

debug_voronoi: voronoi.c debug_heap.o debug_bintree.o
	clang $(FLAGS) -DDEBUG debug_heap.o debug_bintree.o voronoi.c -o debug_voronoi

debug: format debug_test debug_voronoi
	./debug_test > /dev/null
	./debug_voronoi

# end of debug

clean:
	rm -rf *.dSYM *.o *.s
	rm -f heap short_test debug_test voronoi debug_voronoi
