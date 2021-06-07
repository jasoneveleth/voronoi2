#!/bin/sh

# I assume we are in project directory

# pass in the name of the test you want to fix as an argument to this file

RED='\033[0;31m'
GRN='\033[0;32m'
CLR='\033[0m'

if [ "" != "$1" ]; then
    cp tests/sites/hundred_point.gradin input
    .env/bin/python main.py -s -t -n 50 | md5sum > "tests/sites/${1}.gradout"
fi

# ./voronoi <file>
for file in tests/sites/*.in; do
    file_no_extension="$(echo "$file" | cut -f1 -d'.')"
    printf "testing $file_no_extension: "

    build/voronoi "${file_no_extension}.in" | md5sum > tests/tmp_file

    if cmp --silent "${file_no_extension}.out" tests/tmp_file; then
        printf "${GRN}PASSED${CLR}\n"
    else
        printf "${RED}FAILED${CLR}\n"
    fi
done

# gradient descent
# python main.py -s -t -n 50 > tests/sites/hundredpoint_obj=perimeter.gradout
printf "only one of the following descents should pass:\n"
cp tests/sites/hundred_point.gradin input
for file in tests/sites/*.gradout; do
    file_no_extension="$(echo "$file" | cut -f1 -d'.')"
    printf "testing $file_no_extension: "

    .env/bin/python main.py -s -t -n 50 | md5sum > tests/tmp_file

    if cmp --silent "${file_no_extension}.gradout" tests/tmp_file; then
        printf "${GRN}PASSED${CLR}\n"
    else
        printf "${RED}FAILED${CLR}\n"
    fi
done

rm -f tests/tmp_file
