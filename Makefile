FLAGS = -std=c99 -Werror -Weverything -Wno-poison-system-directories
FLAGS += -Ofast
# FLAGS += -g -O0

.PHONY: all run clean test debug

all: heap.o short_test

heap.o: heap.c heap.h
	clang $(FLAGS) -c heap.c -DFLOAT

short_test: test.c heap.o
	clang $(FLAGS) heap.o test.c -o short_test -DFLOAT

test: short_test
	./short_test

# debug

debug.o: heap.c heap.h
	clang $(FLAGS) -DDEBUG -DFLOAT -c heap.c -o debug.o

debug_test: test.c debug.o
	clang  $(FLAGS) -DDEBUG -DFLOAT debug.o test.c -o debug_test

debug: debug_test
	./debug_test

# end of debug

clean:
	rm -rf *.dSYM *.o
	rm -f heap short_test debug_test
