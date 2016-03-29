#!/bin/sh

em++ -std=c++11 ../src/worker/file_traverse.cpp -s EXPORTED_FUNCTIONS="['_Traverse']" -s \
BUILD_AS_WORKER=1 -o ../worker/file_traverse.js
