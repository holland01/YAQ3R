#pragma once

#define ASSET_Q3_ROOT "asset"
#define ASSET_ASCII_DELIMITER "|"

#define LOG_LINE_SEPARATOR "--------------------------\n"

#define C_OFFSETOF_P(p, m) ( uintptr_t( &( ( p )->m ) ) - uintptr_t( ( p ) ) )
#define C_OFFSETOF_T(t, s) C_OFFSETOF_P( ( t* )0x1000, s )

#define SSTREAM_BYTE_OFFSET( T,  name ) "\t" #name ": " << C_OFFSETOF_T( T, name ) << "\n"
#define SSTREAM_BYTE_OFFSET2( T, name ) "\t" SSTREAM_BYTE_OFFSET( T, name )

#define SSTREAM_INFO_PARAM( name ) "\t" #name ": " << ( name ) << ",\n"
#define SSTREAM_INFO_PARAM_OMIT( name ) "\t" #name ": <stubbed out>,\n"
#define SSTREAM_INFO_BEGIN( T ) #T "{\n"
#define SSTREAM_INFO_END() "}\n"

#define SSTREAM_INFO_PARAM_GLM( glmValue ) "\t" #glmValue ": " << ( glm::to_string( glmValue ) ) << ",\n" 

#define SSTREAM_INFO_PARAM_ARRAY2( array ) "\t" #array ": { " << ( array )[ 0 ] << ", " << ( array )[ 1 ] << " }\n"
 
#define MAKE_BASE2_MASK( numBits, shift ) ( ( ( 1 << ( numBits ) ) - 1 ) << ( shift ) )
#define VALUE_FROM_BITS( bits, mask, shift ) ( ( bits & ( mask ) ) >> ( shift ) )
#define SET_BITS_FOR_VALUE( value, bits, mask, shift ) bits &= ~( mask ); bits |= ( ( value ) << ( shift ) ) & ( mask )


#define GLSL_INLINE( v ) #v"\n"

#if defined( EMSCRIPTEN )
#	define EM_SERV_ASSET_PORT "6931"
#endif

#define O_INTERVAL_LOGGING // see io.h

#if defined( DEBUG )
#	define _DEBUG_USE_GL_GET_ERR
#endif

#if defined( _WIN32 )
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

#if defined( _WIN32 )
#   if !defined(NDEBUG)
#       define DEBUG 1
#   endif // NDEBUG
#endif // _WIN32

// Windows.h defines these for us already
#if !defined( _WIN32_ )
#	define TRUE 1
#	define FALSE 0

#	ifdef DEBUG
	static INLINE void __nop(void)
	{
		volatile int a = 0;
		a += 1;
	}
#	endif // DEBUG

#endif // !_WIN32

template< class Tint > static INLINE Tint Align( Tint s )
{ return ( s + Tint( 3 ) ) & ( ~Tint( 3 ) );  }

#ifdef __GNUC__
#	define _FUNC_NAME_ __func__
#	define _LINE_NUM_ __LINE__
#elif defined (_MSC_VER)
#	define _FUNC_NAME_ __FUNCTION__
#	define _LINE_NUM_ __LINE__
#else
#error "Need __GNUC__ or _MSC_VER_ defined"
#endif

#define DEBUG_STUB printf("YAQ3R DEBUG TRACE - %s:%s:%i\n", __FILE__, _FUNC_NAME_, _LINE_NUM_)

#if defined (WEB_WORKER_CLIENT)
// Each of these macros refers to the name of a
// method or function which invokes the async
// web worker API in order to manage resources
// through the Emscripten web worker file system.
//
// We enable/disable these because the upgrade from
// asm.js to actual web assembly is best handled
// through controlled, gradual changes.
#define WEB_WORKER_CLIENT_READMAPFILE_BEGIN
#define WEB_WORKER_CLIENT_READSHADERS
#define WEB_WORKER_CLIENT_SENDREQUEST
#define WEB_WORKER_CLIENT_AIIO_READIMAGES
#define WEB_WORKER_CLIENT_LOADIMAGESEND
#define WEB_WORKER_CLIENT_MAPREADFIN
#define WEB_WORKER_CLIENT_ONSHADERREADFINISH
#endif

#define INDEX_UNDEFINED -1
#define KEY_UNDEFINED "undefined"
#define KEY_DEFINED "active"

#define F_SIZE_T "%zu"
///#define _DEBUG_USE_GL_GET_ERR
#define AABB_MAX_Z_LESS_THAN_MIN_Z // quake 3 maps use this standard in their bounds computations/storage

// From: http://stackoverflow.com/a/4415646/763053
#define UNSIGNED_LEN( x ) ( ( sizeof( x ) / sizeof( 0[x] ) ) / ( ( size_t )( !( sizeof( x ) % sizeof( 0[x] ) ) ) ) )
#define SIGNED_LEN( x ) ( ( int ) ( sizeof( x ) / sizeof( 0[x] ) ) / ( ( int )( !( sizeof( x ) % sizeof( 0[x] ) ) ) ) )

#define ZEROMEM( m ) ( memset( ( m ), 0, sizeof( m ) ) )

#define Mem_Alloc( s ) ( malloc( ( s ) ) )
#define Mem_Free( ptr ) ( free( ( ptr ) ) )

#define IS_ALIGN4( x ) ( ( ( ( x ) >> 2 ) << 2 ) == ( x ) )

#define UNUSED( p ) ( ( void )( p ) )

typedef unsigned int uint;
typedef unsigned char byte;
