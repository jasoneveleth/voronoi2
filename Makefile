ifdef I_WANT_GCC
CC = gcc
FLAGS = -Wall -Wextra
else
CC = clang
FLAGS = -Weverything
endif

PYTHON = .env/bin/python

FLAGS += -std=gnu11 -Werror
FLAGS += -Wno-error=unused-parameter 
FLAGS += -Wno-error=unused-variable 
FLAGS += -Wno-error=unused-function
FLAGS += -Wno-error=double-promotion
FLAGS += -Wno-error=reserved-id-macro
# FLAGS += -Ofast -march=native
FLAGS += -g -O0 

UNAME := $(shell uname)

# linux is annoying and doesn't link math library by default
ifeq ($(UNAME), Linux)
MATH = -lm
endif

# probably shouldn't use Weverthing but this fixes it for mac
ifeq ($(UNAME), Darwin)
FLAGS += -Wno-poison-system-directories -DMAC
endif

SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)
ONLY_FORMAT := $(wildcard src/*.h) tests/heap_test.c

.PHONY: all format clean test lib dirs run

all: format lib

# the leading '-' keeps make from aborting if this fails
format:
	-command -v clang-format >/dev/null 2>&1 && clang-format -i -style=file $(SRC) $(ONLY_FORMAT)

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

bin/voronoi: $(OBJ)
	$(CC) $(FLAGS) $(MATH) $^ -o $@

bin/heap_test: tests/heap_test.c src/heap.o
	$(CC) $(FLAGS) $(MATH) $^ -o $@

test: format bin/voronoi bin/heap_test lib
	bin/heap_test
	sh tests/main_test.sh

clean:
	rm -rf bin/*.dSYM
	rm -f $(OBJ) *.gif bin/* output/*

# could use just `$(PYTHON) setup.py build_ext -i` if you want .so file in cwd
lib:
	$(PYTHON) setup.py build_ext
	$(PYTHON) setup.py install
	rm -rf build/

# env PYTHONMALLOC=malloc valgrind python main.py
run: lib
	$(PYTHON) main.py -g 100
	$(PYTHON) main.py -n 100

