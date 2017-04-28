#!/bin/bash

python3 setup.py build
mv build/lib.*/attacklib_native.cpython-35m-*.so .
