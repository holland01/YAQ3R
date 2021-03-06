#include "context_window.h"
#include "renderer_local.h"
#include "io.h"
#include "glutil.h"

extern "C" {

static std::string PrecisionFormatString( GLenum which )
{
	std::array< GLint, 2 > range;
	GLint precision;
	GL_CHECK( glGetShaderPrecisionFormat( GL_FRAGMENT_SHADER, which, &range[ 0 ], &precision ) );

	std::stringstream ss;

	ss << "( min, max, precision ) = " << "( " << range[ 0 ] << ", " << range[ 1 ] << ", " << precision << " )";

	return ss.str();
}

void GPrintContextInfo( void )
{
	int depthSize;
	int redSize, greenSize, blueSize, alphaSize;

	SDL_GL_GetAttribute( SDL_GL_DEPTH_SIZE, &depthSize );
	SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &redSize );
	SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &greenSize );
	SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &blueSize );
	SDL_GL_GetAttribute( SDL_GL_ALPHA_SIZE, &alphaSize );

	std::stringstream out;
	out << "Buffer Sizes\n"
		<< "\tDepth: " << depthSize << "\n"
		<< "\tRed: " << redSize << "\n"
		<< "\tGreen: " << greenSize << "\n"
		<< "\tBlue: " << blueSize << "\n"
		<< "\tAlpha: " << alphaSize << "\n"
		<< "Fragment shader precisions" << "\n"
		<< "\tLow: " << PrecisionFormatString( GL_LOW_FLOAT ) << "\n"
		<< "\tMedium: " << PrecisionFormatString( GL_MEDIUM_FLOAT ) << "\n"
		<< "\tHigh: " << PrecisionFormatString( GL_HIGH_FLOAT ) << "\n";

	MLOG_INFO_ONCE( "OpenGL Context Info, %s", out.str().c_str() );
}

bool GInitContextWindow( const char* title,
		gContextHandles_t& handles )
{
#ifndef EMSCRIPTEN
	GLenum glewErr = GLEW_OK; // avoid issues with goto
#endif	
	
	if ( SDL_Init( SDL_INIT_VIDEO ) != 0 ) {
		goto sdl_failure;
	}
	
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, G_API_MAJOR_VERSION );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, G_API_MINOR_VERSION );
	
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
	
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, G_API_CONTEXT_PROFILE );

	SDL_CreateWindowAndRenderer( handles.width, handles.height, 
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE, 
			&handles.window, &handles.renderer );

	if ( !handles.window ) 
	{
		goto sdl_failure;	
	}

	SDL_SetWindowTitle( handles.window, title );

	handles.context = SDL_GL_CreateContext( handles.window );

	if ( !handles.context )
	{
		goto sdl_failure;	
	}

#ifndef EMSCRIPTEN
	glewExperimental = true;
	glewErr = glewInit();
	if ( glewErr != GLEW_OK )
	{
		MLOG_ERROR( "Could not initialize GLEW: %s", 
				glewGetErrorString( glewErr ) );
		return false;
	}
#endif

	SDL_RenderPresent( handles.renderer );

	// HACK: error checking is impossible unless this happens;
	// apparently an invalid enumeration exists - possibly
	// in SDL or GLEW's initialization...

	glGetError();

	return true;

sdl_failure:
	MLOG_ERROR( "SDL_Error: %s", SDL_GetError() );
	return false;	
}

} // end extern "C"
