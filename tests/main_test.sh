#!/bin/bash

# I assume: cwd == project directory
# I also assume that everything compiled (i.e. this was called in the make file)

RED='\033[0;31m'
GRN='\033[0;32m'
CLR='\033[0m'

if [ "" != "$1" ]; then
    printf "cp tests/sites/hundred_point.gradin input\n"
    printf "voronoi -n 50 [args]; python plot.py -s -t | md5sum > \"tests/sites/${1}.gradout\"\n"
fi

check_tmp() {
    if cmp --silent "$1" tests/tmp_file; then
        printf "${GRN}PASSED${CLR}\n"
    else
        printf "${RED}FAILED${CLR}\n"
    fi
}

# ========================== ./voronoi <file> ==============================
for file in tests/sites/*.in; do
    file_no_extension="$(echo "$file" | cut -f1 -d'.')"
    printf "testing $file_no_extension: "

    voronoi -n 0 -i "${file_no_extension}.in"
    cat output/sites output/linesegs | md5sum > tests/tmp_file
    check_tmp "${file_no_extension}.out"
done

# =========================== gradient descent =============================
cp tests/sites/hundred_point.gradin input

files=("tests/sites/hundredpoint_obj=perimeter" "tests/sites/hundredpoint_obj=perimeter_repel")
args=("" "-o pr")

for i in ${!files[@]}; do
    printf "testing ${files[$i]} "
    voronoi -n 50 ${args[$i]}
    .env/bin/python plot.py -s -t | md5sum > tests/tmp_file
    check_tmp "${files[$i]}.gradout"
    rm -f tests/tmp_file
done

