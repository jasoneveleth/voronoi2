FLAGS = -std=c11 -Werror -Weverything -Wno-poison-system-directories
# FLAGS += -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function
# FLAGS += -Ofast
FLAGS += -g -O0 
# # makes the debug verbose
# FLAGS += -DDEBUG
FLAGS += -DFLOAT

# linux is annoying and doesn't link math library by default
UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
MATH = -lm
endif

C = bintree.c bintree.h heap.c heap.h voronoi.c voronoi.h tests/heap_test.c

.PHONY: all format clean test

all: format voronoi

format:
	clang-format -i -style=file $C

bintree.o: bintree.c bintree.h
	clang $(FLAGS) -c bintree.c

heap.o: heap.c heap.h
	clang $(FLAGS) -c heap.c

tests/heap_test: tests/heap_test.c heap.o bintree.o
	clang $(FLAGS) $(MATH) heap.o bintree.o tests/heap_test.c -o tests/heap_test

test: format tests/heap_test
	./tests/heap_test
	sh tests/main_test.sh

voronoi: format heap.o bintree.o
	clang $(FLAGS) $(MATH) heap.o bintree.o voronoi.c -o voronoi

clean:
	rm -rf *.dSYM *.o *.s tests/*.dSYM
	rm -f tests/heap_test voronoi
