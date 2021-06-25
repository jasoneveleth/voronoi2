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

.PHONY: all format clean test dirs run

all: format voronoi

# the leading '-' keeps make from aborting if this fails
format:
	-command -v clang-format >/dev/null 2>&1 && clang-format -i -style=file $(SRC) $(ONLY_FORMAT)

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

voronoi: $(OBJ)
	$(CC) $(FLAGS) $(MATH) $^ -o $@

tests/heap_test: tests/heap_test.c src/heap.o
	$(CC) $(FLAGS) $(MATH) $^ -o $@

test: format voronoi tests/heap_test
	tests/heap_test
	sh tests/main_test.sh

clean:
	rm -rf tests/*.dSYM
	rm -f $(OBJ) *.gif output/*

# env PYTHONMALLOC=malloc valgrind python main.py
run: voronoi
	$(PYTHON) plot.py -g 100
	voronoi -t 100 100
	$(PYTHON) plot.py -n 100

