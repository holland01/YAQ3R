#pragma once

#ifdef EMSCRIPTEN

#include "common.h"

void EM_UnmountFS( void );

void EM_MountFS( void );

#endif // EMSCRIPTEN

