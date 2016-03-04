#include "buffer.h"
#include "glutil.h"
#include <stdlib.h>
#include <memory>
#include <stack>

namespace {

struct gVertexBuffer_t
{
	int32_t id = 0;
	GLuint handle = 0;

	~gVertexBuffer_t( void )
	{
		DeleteBufferObject( GL_ARRAY_BUFFER, handle );
	}
};

using vertexBufferPointer_t = std::unique_ptr< gVertexBuffer_t >;

std::vector< vertexBufferPointer_t > gVertexBufferMap;

INLINE std::vector< bspVertex_t > ConvertToDrawVertex( const std::vector< glm::vec3 >& vertices, const std::vector< glm::vec2 >& texCoords )
{
	std::vector< bspVertex_t > bufferData;

	assert( vertices.size() == texCoords.size() && "the size of the vertex data MUST equal the size of the texCoords data" );

	for ( uint32_t i = 0; i < vertices.size(); ++i )
	{
		bspVertex_t vt;
		vt.position = vertices[ i ];
		vt.color = glm::vec4( 1.0f );
		vt.normal = glm::vec3( 1.0f );
		vt.texCoords[ 0 ] = ( i < texCoords.size() )? texCoords[ i ]: glm::vec2( 0.0f );
		vt.texCoords[ 1 ] = glm::vec2( 0.0f );

		bufferData.push_back( vt );
	}

	return bufferData;
}

INLINE gVertexBuffer_t* MakeVertexBuffer_GL( const std::vector< bspVertex_t >& bufferData )
{
	gVertexBuffer_t* buffer = new gVertexBuffer_t();

	GL_CHECK( glGenBuffers( 1, &buffer->handle ) );
	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, buffer->handle ) );
	GL_CHECK( glBufferData( GL_ARRAY_BUFFER, bufferData.size() * sizeof( bspVertex_t ), &bufferData[ 0 ].position[ 0 ], GL_STATIC_DRAW ) );
	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );

	return buffer;
}

} // end namespace

void GEnableDepthBuffer( void )
{
	GL_CHECK( glEnable( GL_DEPTH_TEST ) );
	GL_CHECK( glDepthFunc( GL_LEQUAL ) );
	GL_CHECK( glClearDepth( 1.0f ) );
}

gVertexBufferHandle_t GMakeVertexBuffer( const std::vector< glm::vec3 >& vertices, const std::vector< glm::vec2 >& texCoords )
{
	gVertexBuffer_t* buffer = MakeVertexBuffer_GL( ConvertToDrawVertex( vertices, texCoords ) );

	gVertexBufferHandle_t handle;
	handle.id = ( uint32_t ) gVertexBufferMap.size();
	gVertexBufferMap.push_back( vertexBufferPointer_t( buffer ) );

	return handle;
}

void GFreeVertexBuffer( gVertexBufferHandle_t& handle )
{
	if ( handle.id < gVertexBufferMap.size() )
		gVertexBufferMap.erase( gVertexBufferMap.begin() + handle.id );

	handle.id = G_UNSPECIFIED;
}

void GBindVertexBuffer( const gVertexBufferHandle_t& buffer )
{
	if ( buffer.id < gVertexBufferMap.size() )
		GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, gVertexBufferMap[ buffer.id ]->handle ) );
}

void GReleaseVertexBuffer( void )
{
	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );
}

//------------------------------------------------------

#define _G_INDEX_ALLOC_SIZE 256
#define _G_GROW_RATE( v ) ( ( v ) << 1 )

namespace {
// TODO: implement this using some STL allocators
}

gIndexBufferHandle_t GMakeIndexBuffer( void )
{
	gIndexBufferHandle_t handle;
	handle.id;
	return handle;
}

void GFreeIndexBuffer( gIndexBufferHandle_t buffer )
{
	if ( G_NULL( buffer ) )
	{
		return;
	}

	// TODO: refrain from actually freeing the memory; rather,
	// zero it out and set its length to zero as well. 
	// If we use an STL vector, we shouldn't clear it.
}

void GPushIndex( gIndexBufferHandle_t dest, uint32_t v )
{
	// TODO	
}

uint32_t GGetIndex( gIndexBufferHandle_t buffer, uint32_t index )
{
	// TODO	

	return 0;
}

void GDrawFromIndices( gIndexBufferHandle_t buffer, GLenum mode )
{
	// TODO	
}
