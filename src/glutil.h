#pragma once

#include "common.h"
#include "q3bsp.h"
#include "gldebug.h"
#include "log.h"

// Extensions
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF

#if defined( _DEBUG_USE_GL_ASYNC_CALLBACK )
#	define GL_CHECK( expr )\
		do\
		{\
			( expr );\
			glDebugSetCallInfo( std::string( #expr ), _FUNC_NAME_ );\
		}\
		while ( 0 )
#elif defined( _DEBUG_USE_GL_GET_ERR )
#	define GL_CHECK( expr )\
		do\
		{\
			( expr );\
			ExitOnGLError( _LINE_NUM_, #expr, _FUNC_NAME_ );\
		}\
		while ( 0 )
#else
#	define GL_CHECK( expr ) ( expr )
#endif // _DEBUG_USE_GL_ASYNC_CALLBACK

enum 
{
	GLUTIL_POLYGON_OFFSET_FILL = 1 << 0,
	GLUTIL_POLYGON_OFFSET_LINE = 1 << 1,
	GLUTIL_POLYGON_OFFSET_POINT = 1 << 2
};

static INLINE void LoadVertexLayout( void )
{
	GL_CHECK( glEnableVertexAttribArray( 0 ) );
    GL_CHECK( glEnableVertexAttribArray( 1 ) ); 
    GL_CHECK( glEnableVertexAttribArray( 2 ) );
	GL_CHECK( glEnableVertexAttribArray( 3 ) );

    GL_CHECK( glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( bspVertex_t ), ( void* ) offsetof( bspVertex_t, position ) ) );
    GL_CHECK( glVertexAttribPointer( 1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof( bspVertex_t ), ( void* ) offsetof( bspVertex_t, color ) ) );
    GL_CHECK( glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( bspVertex_t ), ( void* ) offsetof( bspVertex_t, texCoords[ 0 ] ) ) ); // texture
	GL_CHECK( glVertexAttribPointer( 3, 2, GL_FLOAT, GL_FALSE, sizeof( bspVertex_t ), ( void* ) offsetof( bspVertex_t, texCoords[ 1 ] ) ) ); // lightmap
}

static INLINE void LoadBuffer( GLuint vbo )
{
	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, vbo ) );
	LoadVertexLayout();
}

static INLINE void SetPolygonOffsetState( bool enable, uint32_t polyFlags )
{
	if ( enable )
	{
		if ( polyFlags & GLUTIL_POLYGON_OFFSET_FILL ) GL_CHECK( glEnable( GL_POLYGON_OFFSET_FILL ) );
		if ( polyFlags & GLUTIL_POLYGON_OFFSET_LINE ) GL_CHECK( glEnable( GL_POLYGON_OFFSET_LINE ) );
		if ( polyFlags & GLUTIL_POLYGON_OFFSET_POINT ) GL_CHECK( glEnable( GL_POLYGON_OFFSET_POINT ) );
	}
	else
	{
		if ( polyFlags & GLUTIL_POLYGON_OFFSET_FILL ) GL_CHECK( glDisable( GL_POLYGON_OFFSET_FILL ) );
		if ( polyFlags & GLUTIL_POLYGON_OFFSET_LINE ) GL_CHECK( glDisable( GL_POLYGON_OFFSET_LINE ) );
		if ( polyFlags & GLUTIL_POLYGON_OFFSET_POINT ) GL_CHECK( glDisable( GL_POLYGON_OFFSET_POINT ) );
	}
}


bool LoadTextureFromFile( const char* texPath, GLuint texObj, GLuint samplerObj, uint32_t loadFlags, GLenum texWrap );