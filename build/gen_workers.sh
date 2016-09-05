#!/bin/sh

em++ -I../src -std=c++14 ../src/worker/file_traverse.cxx -s  EXPORTED_FUNCTIONS="['_TraverseDirectory', '_ReadFile_Begin', '_ReadFile_Chunk', '_ReadImage', '_UnmountPackages']" -s BUILD_AS_WORKER=1 -s TOTAL_MEMORY=33554432 -s STB_IMAGE=1 -o ../worker/file_traverse.js
