#ifdef  EMSCRIPTEN

#include "em_api.h"
#include "io.h"
#include "js/global_func.h"
#include <html5.h>

#ifdef EM_USE_WORKER_THREAD

worker_t::worker_t( const char* name )
	: handle( emscripten_create_worker( name ) )
{
}

worker_t::~worker_t( void )
{
	emscripten_destroy_worker( handle );
}

static bool gIsLoaded = false;

void worker_t::Await( em_worker_callback_func callback, const char* func, char* data, int size,
	void* param ) const
{
	MLOG_INFO( "Calling Worker ID %i\n", handle );
	int prevQueueSize = emscripten_get_worker_queue_size( handle );
	emscripten_call_worker( handle, func, data, size, callback, param );
}

worker_t gFileWebWorker( "worker/file_traverse.js" );

void EM_FWW_Copy( char* data, int byteSize, void* destVector )
{
	std::vector< unsigned char >& v = *( ( std::vector< unsigned char >* )destVector );
	v.resize( byteSize, 0 );
	memcpy( &v[ 0 ], data, byteSize );
}

void EM_FWW_Dummy( char* data, int byteSize, void* destVector )
{
	UNUSED( data );
	UNUSED( destVector );

	MLOG_INFO( "Worker finished. Byte size of data returned is %i", byteSize );
}

#endif

/*
#define SET_CALLBACK_RESULT( expr )\
	do\
	{\
		ret = ( expr );\
		if ( ret != EMSCRIPTEN_RESULT_SUCCESS )\
		{\
			MLOG_ERROR( "Error setting emscripten function. Result returned: %s, Call expression: %s", EmscriptenResultFromEnum( ret ).c_str(), #expr );\
		}\
	}\
	while( 0 )

INLINE std::string EmscriptenResultFromEnum( int32_t result )
{
	switch ( result )
	{
		case EMSCRIPTEN_RESULT_SUCCESS: return "EMSCRIPTEN_RESULT_SUCCESS";
		case EMSCRIPTEN_RESULT_DEFERRED: return "EMSCRIPTEN_RESULT_DEFERRED";
		case EMSCRIPTEN_RESULT_NOT_SUPPORTED: return "EMSCRIPTEN_RESULT_NOT_SUPPORTED";
		case EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED: "EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED";
		case EMSCRIPTEN_RESULT_INVALID_TARGET: return "EMSCRIPTEN_RESULT_INVALID_TARGET";
		case EMSCRIPTEN_RESULT_UNKNOWN_TARGET: return "EMSCRIPTEN_RESULT_UNKNOWN_TARGET";
		case EMSCRIPTEN_RESULT_INVALID_PARAM: return "EMSCRIPTEN_RESULT_INVALID_PARAM";
		case EMSCRIPTEN_RESULT_FAILED: return "EMSCRIPTEN_RESULT_FAILED";
		case EMSCRIPTEN_RESULT_NO_DATA: return "EMSCRIPTEN_RESULT_NO_DATA";
	}

	return "EMSCRIPTEN_RESULT_UNDEFINED";
}
*/

static bool gMounted = false;

void EM_UnmountFS( void )
{
	if ( gMounted )
	{
		EM_ASM(
			FS.unmount('/memory');
			FS.rmdir('/memory');
		);
		gMounted = false;
	}
}

void EM_MountFS( void )
{
	if ( !gMounted )
	{
		// Load a global definition
		// which can be used from inline javascript
		// snippets
		const char* script =
		   EM_FUNC_WALK_FILE_DIRECTORY"\n"
		   "Module.bspFilesLoaded = false;\n"
		   "Module.walkFileDirectory = walkFileDirectory;\n";

		emscripten_run_script( script );
		EM_ASM(
			FS.mkdir('/memory');
			FS.mount(MEMFS, {}, '/memory');
		);
		gMounted = true;
	}
}
#endif // EMSCRIPTEN
