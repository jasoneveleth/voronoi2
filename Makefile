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
FLAGS += -Wno-poison-system-directories
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

build/voronoi: $(OBJ)
	$(CC) $(FLAGS) $(MATH) $^ -o $@

build/heap_test: tests/heap_test.c src/heap.o
	$(CC) $(FLAGS) $(MATH) $^ -o $@

test: format dirs build/voronoi build/heap_test lib
	build/heap_test
	sh tests/main_test.sh

dirs:
	mkdir -p build

clean:
	rm -rf build/
	rm -f voronoi.cpython* $(OBJ) *.gif

lib:
	$(PYTHON) setup.py build_ext -i

# env PYTHONMALLOC=malloc valgrind python main.py
run: lib
	$(PYTHON) main.py -g 100
	$(PYTHON) main.py -n 100

