#ifdef  EMSCRIPTEN

#include "em_api.h"
#include "io.h"
#include <html5.h>

#ifdef EM_USE_WORKER_THREAD
worker_t gFileWebWorker( "worker/file_traverse.js" );
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
			//FS.unmount(Module['GDEF']['FILE_MEMFS_DIR']);
			//FS.rmdir(Module['GDEF']['FILE_MEMFS_DIR']);
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
		const char* script =
		   "Module.walkFileDirectory = function($0, $1, $2) {\n"
				"var path = UTF8ToString($0);\n"
				"var lookup = FS.lookupPath(path);\n"
				"if (!lookup) {\n"
					"stringToUTF8('Path given ' + path + ' could not be found.', $2, 128);\n"
					"return 0;\n"
				"}\n"
				"var root = lookup.node;\n"
				"var iterate = true;\n"
				"function traverse(node) {\n"
					"if (!iterate) {\n"
						"return;\n"
					"}\n"
					"var path = FS.getPath(node);\n"
					"var stat = FS.stat(path);\n"
					"if (FS.isFile(stat.mode)) {\n"
						"return;\n"
					"}\n"
					"for (var n in node.contents) {\n"
						"traverse(node.contents[n]);\n"
						"var p = FS.getPath(node.contents[n]);\n"
						"var u8buf = intArrayFromString(p);\n"
						"var pbuf = Module._malloc(u8buf.length);\n"
						"Module.writeArrayToMemory(u8buf, pbuf);\n"
						"var stack = Runtime.stackSave();\n"
						"iterate = !!Runtime.dynCall('ii', $1, [pbuf]);\n"
						"Runtime.stackRestore(stack);\n"
						"Module._free(pbuf);\n"
					"}\n"
				"}\n"
				"traverse(root);\n"
				"return 1;\n"
			"};";

		emscripten_run_script( script );
		EM_ASM(
			FS.mkdir('/memory');
			FS.mount(MEMFS, {}, '/memory');
		);
		gMounted = true;
	}
}

#endif // EMSCRIPTEN
