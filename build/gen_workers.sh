#!/bin/sh

em++ -std=c++14 ../src/worker/file_traverse.cxx -s \
EXPORTED_FUNCTIONS="['_TraverseDirectory', '_ReadFile_Begin', '_ReadFile_Chunk']" -s \
BUILD_AS_WORKER=1 -s TOTAL_MEMORY=33554432 -o ../worker/file_traverse.js
