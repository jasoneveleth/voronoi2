#!/bin/sh

# assume we are in project directory

RED='\033[0;31m'
GRN='\033[0;32m'
CLR='\033[0m'

# ./voronoi <file>
for file in tests/sites/*.in; do
    file_no_extension="$(echo "$file" | cut -f1 -d'.')"
    printf "testing $file_no_extension: "

    build/voronoi "${file_no_extension}.in" > tests/tmp_file

    if cmp --silent "${file_no_extension}.out" tests/tmp_file; then
        printf "${GRN}PASSED${CLR}\n"
    else
        printf "${RED}FAILED${CLR}\n"
    fi
done

# gradient descent
printf "testing hundred_points gradient: "
cp tests/sites/hundred_point.gradin input
.env/bin/python main.py -s -t -n 50 > tests/tmp_file
if cmp --silent tests/sites/hundred_point.gradout tests/tmp_file; then
    printf "${GRN}PASSED${CLR}\n"
else
    printf "${RED}FAILED${CLR}\n"
fi

rm -f tests/tmp_file
