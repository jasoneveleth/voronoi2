#!/bin/sh

# I assume: cwd == project directory
# I also assume that everything compiled (i.e. this was called in the make file)

RED='\033[0;31m'
GRN='\033[0;32m'
CLR='\033[0m'

if [ "" != "$1" ]; then
    printf "cp tests/sites/hundred_point.gradin input\n"
    printf ".env/bin/python plot.py -s -t -n 50 [args] | md5sum > \"tests/sites/${1}.gradout\"\n"
fi

# ========================== ./voronoi <file> ==============================
for file in tests/sites/*.in; do
    file_no_extension="$(echo "$file" | cut -f1 -d'.')"
    printf "testing $file_no_extension: "

    voronoi -n 0 -i "${file_no_extension}.in" | md5sum > tests/tmp_file

    if cmp --silent "${file_no_extension}.out" tests/tmp_file; then
        printf "${GRN}PASSED${CLR}\n"
    else
        printf "${RED}FAILED${CLR}\n"
    fi
done

# =========================== gradient descent =============================
cp tests/sites/hundred_point.gradin input

#-----
file_no_extension='tests/sites/hundredpoint_obj=perimeter'
printf "testing ${file_no_extension} "
#-----
voronoi -n 50
.env/bin/python plot.py -s -t -n 50 --objective perimeter --boundary torus --descent constant_alpha | md5sum > tests/tmp_file
if cmp --silent "${file_no_extension}.gradout" tests/tmp_file; then
    printf "${GRN}PASSED${CLR}\n"
else
    printf "${RED}FAILED${CLR}\n"
fi


#----
file_no_extension='tests/sites/hundredpoint_obj=perimeter_repel'
printf "testing ${file_no_extension} "
#----
voronoi -n 50 -o pr
.env/bin/python plot.py -s -t -n 50 --objective repulsion perimeter --boundary torus --descent constant_alpha | md5sum > tests/tmp_file
if cmp --silent "${file_no_extension}.gradout" tests/tmp_file; then
    printf "${GRN}PASSED${CLR}\n"
else
    printf "${RED}FAILED${CLR}\n"
fi

# for file in tests/sites/*.gradout; do
#     file_no_extension="$(echo "$file" | cut -f1 -d'.')"
#     printf "testing $file_no_extension: "

#     .env/bin/python plot.py -s -t -n 50 | md5sum > tests/tmp_file

#     if cmp --silent "${file_no_extension}.gradout" tests/tmp_file; then
#         printf "${GRN}PASSED${CLR}\n"
#     else
#         printf "${RED}FAILED${CLR}\n"
#     fi
# done

rm -f tests/tmp_file
