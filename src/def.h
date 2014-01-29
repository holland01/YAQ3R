#pragma once

/*
==========================

Author: Holland Schutte
License: WTFPL

    def.h

Typedefs and ( known ) necessary
preprocessor directives to maintain multi-platform
compliance.

==========================
*/

#if defined(_WIN32)
#define GL_PROC __stdcall
#elif defined( __GNUC__ ) && defined( __amd64__ )
#define GL_PROC // leave blank; calling convention should be taken care of
#else
#define GL_PROC __attribute__( ( __cdecl ) ) // default to cdecl calling convention on 32-bit non-MSVC compilers
#endif

typedef unsigned int uint;
typedef unsigned char byte;
