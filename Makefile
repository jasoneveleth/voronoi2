FLAGS = -std=c99 -Werror -Weverything -Wno-poison-system-directories
FLAGS += -Ofast
# FLAGS += -g -O0
FLAGS += -DFLOAT
C = heap.c bintree.h heap.h test.c bintree.c

.PHONY: all clean test debug format

all: format short_test

format:
	clang-format -i -style=file $C

bintree.o: bintree.c bintree.h
	# floats are used as keys so need to compare floats
	clang $(FLAGS) -Wno-float-equal -c bintree.c

heap.o: heap.c heap.h
	clang $(FLAGS) -c heap.c

short_test: test.c heap.o bintree.o
	clang $(FLAGS) heap.o bintree.o test.c -o short_test

test: format short_test
	./short_test

# debug

heapdebug.o: heap.c heap.h
	clang $(FLAGS) -DDEBUG -c heap.c -o heapdebug.o

debug_test: test.c heapdebug.o
	clang  $(FLAGS) -DDEBUG heapdebug.o test.c -o debug_test

debug: format debug_test
	./debug_test

# end of debug

clean:
	rm -rf *.dSYM *.o *.s
	rm -f heap short_test debug_test
