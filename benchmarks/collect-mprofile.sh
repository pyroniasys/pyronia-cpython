#!/bin/bash

runs=5

if [ $# -ne 1 ]; then
    echo 'Usage: ./collect-mprofile.sh <benchmark type>'
    exit 0
fi
    
bench_type=$1

apps="alexa"
apps_path='/home/pyronia/libpyronia/apps'
lib_prof_temp='/home/pyronia/cpython/profiles/home.pyronia.cpython.pyronia_build.python-lib.prof'

for a in $apps; do
    cp $lib_prof_temp'.'$a $lib_prof_temp
    /home/pyronia/cpython/load_python_profile
    x=4
    while [ $x -le $runs ]; do
        mprof run --include-children -o $a'/'$a'-mprofile-'$x'-'$bench_type'.data' /home/pyronia/cpython/pyronia_build/python $apps_path'/'$a'/'$a'.py'
        echo "Finished run $x for app $a"
	x=$(($x+1))
    done
done
