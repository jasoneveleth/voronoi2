CC = clang
FLAGS = -std=c11 -Werror -Weverything -Wno-poison-system-directories
FLAGS += -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function
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

C = bintree.c bintree.h heap.c heap.h fortunes.c fortunes.h main.c tests/heap_test.c

.PHONY: all format clean test python

all: format voronoi

format:
	clang-format -i -style=file $C

%.o: %.c
	$(CC) $(FLAGS) -c $<

tests/heap_test: tests/heap_test.c heap.o
	$(CC) $(FLAGS) $(MATH) $^ -o $@

voronoi: main.o heap.o bintree.o fortunes.o
	$(CC) $(FLAGS) $(MATH) $^ -o $@

test: format voronoi tests/heap_test
	./tests/heap_test
	sh tests/main_test.sh

clean:
	rm -rf *.dSYM tests/*.dSYM build/
	rm -f tests/heap_test voronoi *.o *.s voronoi.c voronoi.cpython-39-darwin.so

python:
	. ./.env/bin/activate
	python --version
	env VIRTUAL_ENV="$$PWD/.env" .env/bin/python setup.py build_ext -i
