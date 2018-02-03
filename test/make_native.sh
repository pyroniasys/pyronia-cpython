#!/bin/bash

python3 setup.py build
mv build/lib.*/memtestlib_native.cpython-35m-*.so .
