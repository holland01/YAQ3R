#ifdef  EMSCRIPTEN

#include "em_api.h"

#include <emscripten.h>
#include <html5.h>

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
			FS.unmount('/working');
			FS.rmdir('/working');
		);
		gMounted = false;
	}
}

void EM_MountFS( void )
{
	if ( !gMounted )
	{
		EM_ASM(
			FS.mkdir('/working');
			FS.mount(MEMFS, {}, '/working');
			console.log(FS.stat("asset/stockmaps/maps/Railgun_Arena.bsp"));
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
