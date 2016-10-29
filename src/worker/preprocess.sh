#!/bin/bash

SDL2_ROOT=$HOME/.emscripten_ports/sdl2/SDL2-version_9/
EMMAIN=$EMSCRIPTEN/system/include/emscripten
BASE=$HOME/Devel/yaq3r/src


g++ -DDEBUG -I$BASE -I$EMMAIN -I$SDL2_ROOT -E $1
