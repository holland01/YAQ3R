#pragma once

#define ASSET_Q3_ROOT "asset"

#ifndef EMSCRIPTEN
#	define _DEBUG_USE_GL_GET_ERR
#endif

#if defined(_WIN32)
#	include <Windows.h>
#	define GL_PROC APIENTRY
#	define alloca _alloca
#else
#	define GL_PROC
#endif

#if defined( EMSCRIPTEN ) && defined( ASYNC_FILE_TRAVERSAL )
#	define EM_USE_WORKER_THREAD
#endif

#define INLINE inline

#ifdef _WIN32
#   ifndef NDEBUG
#       define DEBUG 1
#   endif // NDEBUG
#endif // _WIN32

// Windows.h defines these for us already
#ifndef _WIN32
#	define TRUE 1
#	define FALSE 0
#	ifdef DEBUG
static void __nop(void) {}
#	endif
#endif

#ifdef __GNUC__
#	define _FUNC_NAME_ __func__
#	define _LINE_NUM_ __LINE__
#elif defined (_MSC_VER)
#	define _FUNC_NAME_ __FUNCTION__
#	define _LINE_NUM_ __LINE__
#endif

#define INDEX_UNDEFINED -1
#define KEY_UNDEFINED "undefined"
#define KEY_DEFINED "active"

///#define _DEBUG_USE_GL_GET_ERR
#define AABB_MAX_Z_LESS_THAN_MIN_Z // quake 3 maps use this standard in their bounds computations/storage

// From: http://stackoverflow.com/a/4415646/763053
#define UNSIGNED_LEN( x ) ( ( sizeof( x ) / sizeof( 0[x] ) ) / ( ( size_t )( !( sizeof( x ) % sizeof( 0[x] ) ) ) ) )
#define SIGNED_LEN( x ) ( ( int ) ( sizeof( x ) / sizeof( 0[x] ) ) / ( ( int )( !( sizeof( x ) % sizeof( 0[x] ) ) ) ) )

#define ZEROMEM( m ) ( memset( ( m ), 0, sizeof( m ) ) )

#define Mem_Alloc( s ) ( malloc( ( s ) ) )
#define Mem_Free( ptr ) ( free( ( ptr ) ) )

#define UNUSED( p ) ( void )p

typedef unsigned int uint;
typedef unsigned char byte;
