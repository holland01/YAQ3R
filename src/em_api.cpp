#ifdef  EMSCRIPTEN

#include "em_api.h"
#include <html5.h>

#ifdef EM_USE_WORKER_THREAD
worker_t gFileWebWorker( "worker/file_traverse.js" );
#endif

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

static bool gMounted = false;

void EM_UnmountFS( void )
{
	if ( gMounted )
	{
		EM_ASM(
			FS.unmount(Module['GDEF']['FILE_MEMFS_DIR']);
			FS.rmdir(Module['GDEF']['FILE_MEMFS_DIR']);
		);
		gMounted = false;
	}
}

void EM_MountFS( void )
{
	if ( !gMounted )
	{
		EM_ASM(
			if (!Module['GDEF']) {
				Module['GDEF'] = {};
			}
			Module['GDEF']['FILE_MEMFS_DIR'] = '/memory';

			Module['GFUNC_WALKDIR'] = function(directory, callback, error) {
				var path = UTF8ToString(directory);
				var lookup = FS.lookupPath(path);
				if (!lookup) {
					stringToUTF8('Path given ' + path + ' could not be found.', error, 128);
					return 0;
				}

				var root = lookup.node;
				var iterate = true;

				function traverse(node) {
					if (!iterate) return;

					var path = FS.getPath(node);
					var stat = FS.stat(path);
					if (FS.isFile(stat.mode)) {
						return;
					}

					for (var n in node.contents) {
						traverse(node.contents[n]);
						var p = FS.getPath(node.contents[n]);
						var u8buf = intArrayFromString(p);
						var pbuf = Module._malloc(u8buf.length);
						Module.writeArrayToMemory(u8buf, pbuf);
						var stack = Runtime.stackSave();
						iterate = !!Runtime.dynCall('ii', callback, [pbuf]);
						Runtime.stackRestore(stack);
						Module._free(pbuf);
					}
				}

				traverse(root);
				return 1;
			}

			FS.mkdir('/memory');
			FS.mount(MEMFS, {}, '/memory');
		);
		gMounted = true;
	}

	/*
	int32_t ret;
	SET_CALLBACK_RESULT( emscripten_set_keydown_callback( nullptr, nullptr, 0, ( em_key_callback_func )&KeyInputFunc< &input_client::EvalKeyPress > ) );
	SET_CALLBACK_RESULT( emscripten_set_keyup_callback( nullptr, nullptr, 0, ( em_key_callback_func )&KeyInputFunc< &input_client::EvalKeyRelease > ) );
	SET_CALLBACK_RESULT( emscripten_set_mousemove_callback( "#canvas", nullptr, 1, ( em_mouse_callback_func )&MouseMoveFunc ) );
	SET_CALLBACK_RESULT( emscripten_set_mousedown_callback( "#canvas", nullptr, 1, ( em_mouse_callback_func )&MouseDownFunc ) );
	*/
}

#endif // EMSCRIPTEN
