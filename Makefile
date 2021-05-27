FLAGS = -std=c11 -Werror -Weverything -Wno-poison-system-directories

# linux is annoying and doesn't link math library by default
UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
MATH = -lm
endif

FLAGS += -Ofast
# FLAGS += -g -O0 -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function
# makes the debug verbose
# FLAGS += -DDEBUG
FLAGS += -DFLOAT
C = bintree.c bintree.h heap.c heap.h test.c voronoi.c voronoi.h

.PHONY: all clean test debug format

all: format short_test voronoi

format:
	clang-format -i -style=file $C

bintree.o: bintree.c bintree.h
	clang $(FLAGS) -c bintree.c

heap.o: heap.c heap.h
	clang $(FLAGS) -c heap.c

short_test: test.c heap.o bintree.o
	clang $(FLAGS) $(MATH) heap.o bintree.o test.c -o short_test

test: format short_test
	./short_test

voronoi: format heap.o bintree.o
	clang $(FLAGS) $(MATH) heap.o bintree.o voronoi.c -o voronoi

clean:
	rm -rf *.dSYM *.o *.s
	rm -f heap short_test debug_test voronoi debug_voronoi
