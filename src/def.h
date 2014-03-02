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
#define GL_PROC // leave blank: calling convention should be taken care of on this architecture
#else
#define GL_PROC __attribute__( ( __cdecl ) ) // default to cdecl calling convention on 32-bit non-MSVC compilers
#endif

#ifndef __GNUC__
#include <stdint.h>
#else
#include <stdint-gcc.h>
#endif

#define INLINE inline
#define GLM_FORCE_RADIANS

typedef unsigned int uint;
typedef unsigned char byte;

extern void* AllocMem( unsigned long sz );
extern void  FreeMem( void* ptr );

// From: http://stackoverflow.com/a/4415646/763053 (originally named "COUNT_OF")
#define UNSIGNED_LEN(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#define SIGNED_LEN(x) ((int)((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x]))))))

#define BUFFER_OFFSET( type, member ) ( ( void* ) offsetof( type, member ) )

#define Mem_Alloc( s ) ( AllocMem( ( s ) ) )
#define Mem_Free( ptr ) ( FreeMem( ( ptr ) ) )
