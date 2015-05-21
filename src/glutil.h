#pragma once

#include "common.h"
#include "q3bsp.h"
#include "gldebug.h"
#include "log.h"
#include <array>

#define UBO_TRANSFORMS_BLOCK_BINDING 0
#define ATTRIB_OFFSET( type, member )( ( void* ) offsetof( type, member ) ) 

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

static INLINE void MapAttribTexCoord( int location, size_t offset )
{
	GL_CHECK( glEnableVertexAttribArray( location ) );
	GL_CHECK( glVertexAttribPointer( location, 2, GL_FLOAT, GL_FALSE, sizeof( bspVertex_t ), ( void* ) offset ) );
}

static INLINE void LoadVertexLayout( void )
{
	GL_CHECK( glEnableVertexAttribArray( 0 ) );
    GL_CHECK( glEnableVertexAttribArray( 1 ) ); 

    GL_CHECK( glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( bspVertex_t ), ( void* ) offsetof( bspVertex_t, position ) ) );
    GL_CHECK( glVertexAttribPointer( 1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof( bspVertex_t ), ( void* ) offsetof( bspVertex_t, color ) ) );
    
	MapAttribTexCoord( 2, offsetof( bspVertex_t, texCoords[ 0 ] ) ); // texture
	MapAttribTexCoord( 3, offsetof( bspVertex_t, texCoords[ 1 ] ) ); // lightmap
}

static INLINE void MapUniforms( glHandleMap_t& unifMap, GLuint programID, const std::vector< std::string >& uniforms )
{
	for ( const std::string& title: uniforms )
	{
		GLint uniform;
		GL_CHECK( uniform = glGetUniformLocation( programID, title.c_str() ) );
		unifMap.insert( glHandleMapEntry_t( title, uniform ) );
	}
}

static INLINE void MapProgramToUBO( GLuint programID, const char* uboName )
{
	if ( strcmp( uboName, "Transforms" ) == 0 )
	{
		GLuint uniformBlockLoc;
		GL_CHECK( uniformBlockLoc = glGetUniformBlockIndex( programID, uboName ) );
		GL_CHECK( glUniformBlockBinding( programID, uniformBlockLoc, UBO_TRANSFORMS_BLOCK_BINDING ) );
	}
}

static INLINE GLuint GenVertexArrayObject( void )
{
	GLuint vao;
	GL_CHECK( glGenVertexArrays( 1, &vao ) );
	return vao;
}

static INLINE GLuint GenBufferObject( GLenum target, const size_t size, const void* data, GLenum usage )
{
	GLuint obj;
	GL_CHECK( glGenBuffers( 1, &obj ) );
	GL_CHECK( glBindBuffer( target, obj ) );
	GL_CHECK( glBufferData( target, size, data, usage ) );
	GL_CHECK( glBindBuffer( target, 0 ) );
	return obj;
}

static INLINE void UpdateBufferObject( GLuint obj, GLenum target, const size_t size, const void* data ) 
{
	GL_CHECK( glBindBuffer( target, obj ) );
	GL_CHECK( glBufferSubData( target, 0, size, data ) );
	GL_CHECK( glBindBuffer( target, 0 ) );
}

static INLINE void DelBufferObject( GLenum target, GLuint* obj, size_t numObj )
{
	// Unbind to prevent driver from lazy deletion
	GL_CHECK( glBindBuffer( target, 0 ) );
	GL_CHECK( glDeleteBuffers( numObj, obj ) );
}

static INLINE void LoadBufferLayout( GLuint vbo )
{
	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, vbo ) );
	LoadVertexLayout();
}

static INLINE void DrawElementBuffer( GLuint ibo, size_t numIndices )
{
	GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo ) );
	GL_CHECK( glDrawElements( GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr ) );
	GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) );
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

static INLINE void ImPrep( const glm::mat4& viewTransform, const glm::mat4& clipTransform )
{
	GL_CHECK( glUseProgram( 0 ) );
	GL_CHECK( glMatrixMode( GL_PROJECTION ) );
	GL_CHECK( glLoadIdentity() );
	GL_CHECK( glLoadMatrixf( glm::value_ptr( clipTransform ) ) );
	GL_CHECK( glMatrixMode( GL_MODELVIEW ) );
	GL_CHECK( glLoadIdentity() );
	GL_CHECK( glLoadMatrixf( glm::value_ptr( viewTransform ) ) );
}

static INLINE void ImDrawAxes( const float size ) 
{
	std::array< glm::vec3, 6 > axes = 
	{
		glm::vec3( size, 0.0f, 0.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, size, 0.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ),
		glm::vec3( 0.0f, 0.0f, -size ), glm::vec3( 0.0f, 0.0f, 1.0f )
	};
	
	glBegin( GL_LINES );
	for ( int i = 0; i < 6; i += 2 )
	{
		glColor3fv( glm::value_ptr( axes[ i + 1 ] ) );
		glVertex3f( 0.0f, 0.0f, 0.0f );
		glVertex3fv( glm::value_ptr( axes[ i ] ) ); 
	}
	glEnd();
}

bool LoadTextureFromFile( const char* texPath, GLuint texObj, GLuint samplerObj, uint32_t loadFlags, GLenum texWrap );