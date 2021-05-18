#!/bin/sh

if ! [ -x "`which clang-format`" ]; then
    exit
fi

find `pwd -P` -regex ".*\.[ch]$" | while read file; do
    echo "$file"
    clang-format -i -style=file "$file"
done

