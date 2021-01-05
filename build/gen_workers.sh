#!/bin/bash

# Pass -DDEBUG before -I to enable console info 
# -s "EXPORTED_RUNTIME_METHODS=['stackSave', 'stackRestore', 'dynCall']"
em++ -I$PWD/../src -fno-inline-functions -O0 -std=c++14 ../src/worker/file_traverse.cxx\
     -s "EXTRA_EXPORTED_RUNTIME_METHODS=['writeStringToMemory', 'dynCall']"\
     -s "EXPORTED_FUNCTIONS=['_ReadShaders', '_ReadMapFile_Begin', '_ReadMapFile_Chunk', '_ReadImage', '_MountPackage', '_UnmountPackages']"\
     -s BUILD_AS_WORKER=1\
     -s TOTAL_MEMORY=234881024\
     -s STB_IMAGE=1\
     -lworkerfs.js -o ../worker/file_traverse.js
