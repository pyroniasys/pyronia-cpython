#!/bin/bash

APP=$1

LD_PRELOAD=/lib/x86_64-linux-gnu/libpyronia_opt.so /home/pyronia/cpython/pyronia_build/python /home/pyronia/libpyronia/apps/$APP/$APP.py
