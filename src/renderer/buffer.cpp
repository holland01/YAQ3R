#include "buffer.h"
#include "glutil.h"
#include <stdlib.h>
#include <memory>

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

	gVertexBufferHandle_t handle =
	{
		( uint32_t ) gVertexBufferMap.size()
	};

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
#define _G_GROW_RATE( v ) ( ( v ) + ( ( v ) >> 3 ) )

namespace {
	struct gIndexListMemory_t
	{
		uint8_t** indexLists;
		uint32_t* listSizes;	// in bytes
		uint32_t* capacities; // in bytes

		// represents the byte level for each buffer, where the amount of bytes taken up by each element is 2^powerLevel. 
		// Cue DBZ joke...
		uint8_t * powerLevels;

		uint16_t  numLists;

		uint16_t  capacity;		// the amount of address space for this memory manager

		gIndexListMemory_t( void )
			: indexLists( new uint8_t*[ _G_INDEX_ALLOC_SIZE ]() ),
			  listSizes( new uint32_t[ _G_INDEX_ALLOC_SIZE ]() ),
			  capacities( new uint32_t[ _G_INDEX_ALLOC_SIZE ]() ),
			  powerLevels( new uint8_t[ _G_INDEX_ALLOC_SIZE ]() ),
			  numLists( 0 ),
			  capacity( _G_INDEX_ALLOC_SIZE )
		{
		}

		~gIndexListMemory_t( void )
		{
			if ( indexLists  )
			{
				for ( uint16_t i = 0; i < numLists; ++i )
					if ( indexLists[ i ] )
						delete[] indexLists[ i ];
				delete[] indexLists;
			}

			if ( listSizes )
				delete[] listSizes;

			if ( powerLevels )
				delete[] powerLevels;

			if ( capacities )
				delete[] capacities;
		}

		template < class Tbuffer >
		Tbuffer* Realloc( Tbuffer* mem, size_t currSize, size_t newSize )
		{
			Tbuffer* newBlock = new Tbuffer[ newSize ]();
			memcpy( newBlock, mem, sizeof( mem[ 0 ] ) * glm::min( currSize, newSize ) );
			delete[] mem;
			return newBlock;
		}

		// Doubles the byte width for every element within the list
		// represented by param 'id'. 
		void RealignList( size_t id )
		{
			uint8_t oldAlignment = powerLevels[ id ];
			uint32_t numIndices = listSizes[ id ] >> powerLevels[ id ];
			
			powerLevels[ id ]++; // Upgrade our power level...
			uint8_t alignment = powerLevels[ id ];

			uint8_t* buffer = indexLists[ id ];
			buffer = Realloc( buffer, capacities[ id ], capacities[ id ] << 1 );

			uint8_t prevStride = 1 << oldAlignment;

			for ( uint32_t k = numIndices - 1; k >= 0; --k )
			{
				memmove( &buffer[ k << alignment ], &buffer[ k << oldAlignment ], prevStride );
				memset( &buffer[ k << oldAlignment ], 0, prevStride ); // zero out the area we just copied, to ensure integrity
			}
			
			listSizes[ id ] <<= 1;
			indexLists[ id ] = buffer;
		}

		uint32_t GenList( void )
		{
			uint32_t id = numLists++;
			uint8_t* buffer = new uint8_t[ _G_INDEX_ALLOC_SIZE ]();

			if ( numLists == capacity )
			{
				size_t newCap = _G_GROW_RATE( capacity );
				indexLists = Realloc( indexLists, capacity, newCap );
				listSizes = Realloc( listSizes, capacity, newCap );
				powerLevels = Realloc( powerLevels, capacity, newCap );
				capacity = newCap;
			}

			indexLists[ id ] = buffer;
			listSizes[ id ] = 0;
			capacities[ id ] = _G_INDEX_ALLOC_SIZE;
			powerLevels[ id ] = 0;

			return id;
		}

		void AddValue( uint32_t id, uint32_t v )
		{
			uint32_t currBitLength = 8 << powerLevels[ id ];

			if ( currBitLength == 32 )
				--currBitLength;

			// Grow width as necessary
			while ( v >= ( 1 << currBitLength ) )
				RealignList( id );

			if ( listSizes[ id ] == capacities[ id ] )
				 indexLists[ id ] = Realloc( indexLists[ id ], capacities[ id ], _G_GROW_RATE( capacities[ id ] ) );

			indexLists[ id ][ listSizes[ id ] << powerLevels[ id ] ] = v;
			listSizes[ id ]++;
		}
	};

	gIndexListMemory_t gIndexMem;
}

gIndexBufferHandle_t GMakeIndexBuffer( void )
{
	gIndexBufferHandle_t handle = 
	{
		gIndexMem.GenList()
	};

	return handle;
}

void GIndexBufferAdd( gIndexBufferHandle_t dest, uint32_t v )
{
	gIndexMem.AddValue( dest.id, v );
}