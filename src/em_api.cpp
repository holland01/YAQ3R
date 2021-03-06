#ifdef  EMSCRIPTEN

#include "em_api.h"
#include "io.h"
#include "js/global_func.h"
#include <emscripten/html5.h>

#ifdef EM_USE_WORKER_THREAD

worker_t::worker_t( const char* name )
	: handle( emscripten_create_worker( name ) )
{
}

worker_t::~worker_t( void )
{
	emscripten_destroy_worker( handle );
}

void worker_t::Await( em_worker_callback_func callback, const char* func, char* data,
	int size, void* param ) const
{
	emscripten_call_worker( handle, func, data, size, callback, param );
}

void worker_t::Await( em_worker_callback_func callback, const char* func,
	const std::string& strData, void* param ) const
{
	std::vector< char > dupe( strData.size() + 1, 0 );
	memcpy( &dupe[ 0 ], strData.c_str(), strData.size() );
	Await( callback, func, &dupe[ 0 ], dupe.size(), param );
}

worker_t gFileWebWorker( "worker/file_traverse.js" );

void EM_FWW_Copy( char* data, int byteSize, void* destVector )
{
	std::vector< unsigned char >& v = *( ( std::vector< unsigned char >* )destVector );
	v.resize( byteSize, 0 );
	memcpy( &v[ 0 ], data, byteSize );
	//MLOG_INFO( "Job's finished. Vector Size: %i bytes. Source Size: %i bytes",
	//	v.size(), byteSize );
}

void EM_FWW_Dummy( char* data, int byteSize, void* destVector )
{
	UNUSED( data );
	UNUSED( destVector );
	UNUSED( byteSize );

//	MLOG_INFO( "Worker finished. Byte size of data returned is %i", byteSize );
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
		EM_ASM({
			FS.unmount('/memory');
			FS.rmdir('/memory');
			}, 0);
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
		const char* walkFileDirectory  =
		   EM_FUNC_WALK_FILE_DIRECTORY"\n"
		   "Module.walkFileDirectory = walkFileDirectory;\n";

		emscripten_run_script( walkFileDirectory );

		const char* printHeapString = "\n"\
			"function printHeapString(address) {\n"\
			"\tvar counter = address;\n"\
			"\tvar str = '';\n"\
			"\twhile (HEAP8[counter] != 0) {\n"\
			"\t\tstr += String.fromCharCode(HEAP8[counter++]);\n"\
			"\t}\n"\
			"\tconsole.log(str);\n"\
			"}\n"\
			"Module.printHeapString = printHeapString;\n";

		emscripten_run_script( printHeapString );

		/*EM_ASM(
			FS.mkdir('/memory');
			FS.mount(MEMFS, {}, '/memory');
		);
		gMounted = true;
		*/
	}
}
#endif // EMSCRIPTEN
