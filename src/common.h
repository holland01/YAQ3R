#pragma once

#if defined( _WIN32 )
#	include <Windows.h> // This needs to be before GLFW includes to prevent APIENTRY macro redef error
#	define GL_PROC APIENTRY
#elif defined( __GNUC__ ) && defined( __amd64__ )
#	define GL_PROC // leave blank: calling convention should be taken care of on this architecture
#else
#	define GL_PROC __attribute__( ( __cdecl ) ) // default to cdecl calling convention on 32-bit non-MSVC compilers
#endif
#ifdef __GNUC__
#	include <stdint-gcc.h>
#else
#	include <stdint.h>
#endif

#include "global.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <time.h>

#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include <map>
#include <utility>
#include <stack>

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#define INLINE inline

// Windows.h defines these for us already
#ifndef _WIN32
#	define TRUE 1 
#	define FALSE 0

static void __nop( void )
{}
#endif

#define _DEBUG_USE_GL_GET_ERR
#define AABB_MAX_Z_LESS_THAN_MIN_Z // quake 3 maps use this standard in their bounds computations/storage

// From: http://stackoverflow.com/a/4415646/763053
#define UNSIGNED_LEN( x ) ( ( sizeof( x ) / sizeof( 0[x] ) ) / ( ( size_t )( !( sizeof( x ) % sizeof( 0[x] ) ) ) ) )
#define SIGNED_LEN( x ) ( ( int ) ( sizeof( x ) / sizeof( 0[x] ) ) / ( ( int )( !( sizeof( x ) % sizeof( 0[x] ) ) ) ) )

#define Mem_Alloc( s ) ( malloc( ( s ) ) )
#define Mem_Free( ptr ) ( free( ( ptr ) ) )

typedef unsigned int uint;
typedef unsigned char byte;

using glHandleMap_t = std::map< std::string, GLint >;
using glHandleMapEntry_t = std::pair< std::string, GLint >;

INLINE bool FileGetExt( std::string& outExt, const std::string& filename  )
{
	// Second condition is to ensure we actually have a file extension we can use
	size_t index;
	if ( ( index = filename.find_last_of( '.' ) ) != std::string::npos && index != filename.size() - 1 )
	{
		outExt = filename.substr( index + 1 );
		return true;
	}
    return false;
}
