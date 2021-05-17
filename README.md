### Notes

when I'm trying to make a pointer to an element in the heap, rather than make a 
pointer to that place in memory (the place in the array), just take the pointer 
to the value - if you need the place in memory, just use the index as an offset 
from the heap's arr pointer.

Need `clang-format` in PATH
