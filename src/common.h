#pragma once

/*
#if defined( _WIN32 )
#	include <Windows.h> // This needs to be before GLFW includes to prevent APIENTRY macro redef error
#	define GL_PROC APIENTRY
#elif defined( __GNUC__ ) && defined( __amd64__ )
#	define GL_PROC // leave blank: calling convention should be taken care of on this architecture
#else
#	define GL_PROC __attribute__( ( __cdecl ) ) // default to cdecl calling convention on 32-bit non-MSVC compilers
#endif
*/

#include "commondef.h"

#include <alloca.h>

#ifdef EMSCRIPTEN
#	include <GLES2/gl2.h>
#	include <GLES2/gl2ext.h>
#	include <EGL/egl.h>
#else
#	include <GL/glew.h>
#endif

#ifdef __linux__
#   include <ftw.h>
#endif

#include <stdint.h>

#include "global.h"

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

// If there's no native handler definition
// which causes DEBUG to be defined for debug builds,
// we define it through the build system directly.
// (the following NDEBUG check is more or less for clarity - i.e.,
// DON'T use NDEBUG to verify that a debug build is being used)

#ifdef _MSC_VER
#	define GLM_FORCE_RADIANS 1
#endif // _WIN32

#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// generic function pointer handler for
// async-oriented function calls
typedef void ( *onFinishEvent_t )( void );

using glHandleMap_t = std::map< std::string, GLint >;
using glHandleMapEntry_t = std::pair< std::string, GLint >;
