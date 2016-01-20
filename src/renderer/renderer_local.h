#pragma once

#define G_HANDLE_INVALID 0xDEADBEEF

#ifdef EMSCRIPTEN
#   define glClearDepth glClearDepthf
#endif

