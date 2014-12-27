#pragma once

#include "common.h"
#include "q3bsp.h"
#include "gldebug.h"

#ifdef _DEBUG
#	define GL_CHECK( expr ) \
		do					\
		{					\
			( expr );		\
			glDebugSetCallInfo( std::string( #expr ), _FUNC_NAME_ );	\
		}					\
		while ( 0 )
#else
#	define GL_CHECK( expr ) ( expr )
#endif

INLINE void LoadVertexLayout( void )
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

INLINE void LoadBuffer( GLuint vbo )
{
	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, vbo ) );
	LoadVertexLayout();
}