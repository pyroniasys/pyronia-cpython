#!/bin/bash

# This script builds a Pyronia-aware version of cpython, which can be
# installed on the system side-by-side vanilla Python.
# author: Marcela S. Melara

ulimit -c unlimited
rm -rf pyronia_build
rm -f core
mkdir -p pyronia_build

./load_python_profile

cd pyronia_build

../configure --enable-unicode=ucs4 --with-libs='-lnl-3 -lnl-genl-3 -lsmv -lpyronia' --with-pyronia --with-pyronia-benchmarking
make
