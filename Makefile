ifdef I_WANT_GCC
CC = gcc
FLAGS = -Wall -Wextra
else
CC = clang
FLAGS = -Weverything
endif

PYTHON = .env/bin/python

FLAGS += -std=c11 -Werror
FLAGS += -Wno-error=unused-parameter 
FLAGS += -Wno-error=unused-variable 
FLAGS += -Wno-error=unused-function
FLAGS += -Wno-error=double-promotion
FLAGS += -Wno-reserved-id-macro
FLAGS += -Wno-format-nonliteral
FLAGS += -g -O3 -march=native
# FLAGS += -pg
FLAGS += -DNTHREADS=16
LINKER = -lpthread

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
OUTPUTDIR := output

.PHONY: all format clean test run

all: format voronoi

# the leading '-' keeps make from aborting if this fails
format:
	-clang-format -i -style=file $(SRC) $(ONLY_FORMAT)

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

# the leading '@' means don't echo the command
$(OUTPUTDIR):
	@mkdir -p $@

voronoi: $(OBJ) | $(OUTPUTDIR)
	$(CC) $(FLAGS) $(LINKER) $(MATH) $^ -o $@

tests/heap_test: tests/heap_test.c src/heap.o
	$(CC) $(FLAGS) $(MATH) $^ -o $@

test: format voronoi tests/heap_test
	rm -f output/*
	tests/heap_test
	bash tests/main_test.sh

clean:
	rm -rf tests/*.dSYM
	rm -f $(OBJ) *.gif output/*

run: voronoi
	$(PYTHON) plot.py -g 100
	voronoi -n 100
	$(PYTHON) plot.py

