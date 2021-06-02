CC = clang

FLAGS = -std=c11 -Werror -Weverything
# FLAGS += -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function
# FLAGS += -Ofast
FLAGS += -g -O0 
# FLAGS += -DDEBUG

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

.PHONY: all format clean test python dirs

all: format dirs build/voronoi

format:
	# the leading - cause 'make' to not fail when this fails
	-command -v clang-format && clang-format -i -style=file $(SRC) $(ONLY_FORMAT)

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

build/voronoi: $(OBJ)
	$(CC) $(FLAGS) $(MATH) $^ -o $@

build/heap_test: tests/heap_test.c src/heap.o
	$(CC) $(FLAGS) $(MATH) $^ -o $@

test: format dirs build/voronoi build/heap_test
	build/heap_test
	sh tests/main_test.sh

dirs:
	mkdir -p build

clean:
	rm -rf build/
	rm -f voronoi.cpython* $(OBJ)

python:
	. ./.env/bin/activate
	env VIRTUAL_ENV="$$PWD/.env" .env/bin/python setup.py build_ext -i
	# env PYTHONMALLOC=malloc valgrind python main.py
