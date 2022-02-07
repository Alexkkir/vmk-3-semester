#!/bin/bash
cd build
mingw32-make
cd ..
python tests/run.py