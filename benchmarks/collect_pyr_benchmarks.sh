#!/bin/bash

runs=2

if [ $# -ne 1 ]; then
    echo 'Usage: ./collect_pyr_benchmarks.sh <benchmark type>'
    exit 0
fi
    
bench_type=$1

apps="twitterPhoto"
apps_path='/home/pyronia/libpyronia/apps'
lib_prof_temp='/home/pyronia/cpython/profiles/home.pyronia.cpython.pyronia_build.python-lib.prof'

for a in $apps; do
    cp $lib_prof_temp'.'$a $lib_prof_temp
    ../load_python_profile
    x=1
    while [ $x -le $runs ]; do
	export LD_PRELOAD=/lib/x86_64-linux-gnu/libpyronia_opt.so
        ../pyronia_build/python $apps_path'/'$a'/'$a'.py'
        echo "Finished run $x for app $a"
	x=$(($x+1))
	../load_python_profile
    done
    mv $apps_path'/'$a'/'$a'.py.data' $a'/'$a'-'$bench_type'.data'
done
