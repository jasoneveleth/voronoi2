#!/bin/sh

if ! [ -x "`which clang-format`" ]; then
    exit
fi

find `pwd` -regex ".*\.[ch]$" | while read file; do
    clang-format -i -style='.clang-format' "$file"
done

