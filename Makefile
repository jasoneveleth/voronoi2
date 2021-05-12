FLAGS = -Ofast -std=c99 -Werror -Weverything -Wno-poison-system-directories

.PHONY: all run clean

all: heap.o test run

heap.o: heap.c heap.h
	clang $(FLAGS) -c heap.c

test: test.c heap.o
	clang $(FLAGS) heap.o test.c -o test

run:
	./test

clean:
	rm -rf *.dSYM *.o
	rm -f heap test
