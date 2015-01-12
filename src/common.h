#pragma once

/*
==========================

Author: Holland Schutte
License: WTFPL

    common.h

Global include file, containing often-used or down-right-necessary files for each module.

==========================
*/

#include <Windows.h> // This needs to be before GLFW includes to prevent APIENTRY macro redef error

#if defined( _WIN32 )
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

#define INLINE inline

typedef unsigned int uint;
typedef unsigned char byte;

#define TRUE 1
#define FALSE 0

#define _DEBUG_USE_GL_GET_ERR

// From: http://stackoverflow.com/a/4415646/763053 (originally named "COUNT_OF")
#define UNSIGNED_LEN(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#define SIGNED_LEN(x) ((int)((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x]))))))

#define Mem_Alloc( s ) ( malloc( ( s ) ) )
#define Mem_Free( ptr ) ( free( ( ptr ) ) )

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
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "vec.h"

INLINE bool FileGetExt( std::string& outExt, const std::string& filename  )
{
	// Second condition is to ensure we actually have a file extension we can use
	size_t index;
	if ( ( index = filename.find_last_of( '.' ) ) != std::string::npos && index != filename.size() - 1 )
	{
		outExt = filename.substr( index + 1 );
		return true;
	}
	return index >= 0;
}
