#include "bezpatch.h"
#include "log.h"
#include "q3bsp.h"
#include "glutil.h"

BezPatch::BezPatch( GLuint clientVbo_, GLuint clientVao_ )
	: clientVbo( clientVbo_ ),
	  clientVao( clientVao_ )
{
	GL_CHECK( glGenBuffers( BEZ_BUF_COUNT, buffers ) ); 
	GL_CHECK( glGenVertexArrays( 1, &vao ) );

	//memset( controlPoints, 0, sizeof( const bspVertex_t* ) * BEZ_CONTROL_POINT_COUNT );  
}

BezPatch::~BezPatch( void )
{
	GL_CHECK( glDeleteBuffers( BEZ_BUF_COUNT, buffers ) );
	GL_CHECK( glDeleteVertexArrays( 1, &vao ) );

	// Rebind on destruct so we don't have to do it ourselves after using this class
	GL_CHECK( glBindVertexArray( clientVao ) );
	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, clientVbo ) );
}

// From Paul Baker's Octagon project, as referenced in http://graphics.cs.brown.edu/games/quake/quake3.html
void BezPatch::Tesselate( int level )
{
	// Vertex count along a side is 1 + number of edges
    const int L1 = level + 1;

	vertices.resize( L1 * L1 );
	
	// Compute the first spline along the edge
	for ( int i = 0; i <= level; ++i )
	{
		float a = ( float )i / ( float )level;
		float b = 1.0f - a;

		vertices[ i ] = 
			*( controlPoints[ 0 ] ) * ( b * b ) +
		 	*( controlPoints[ 3 ] ) * ( 2 * b * a ) + 
			*( controlPoints[ 6 ] ) * ( a * a );
	}

	// Go deep and fill in the gaps; outer is the first layer of curves
	for ( int i = 1; i <= level; ++i )
	{
		float a = ( float )i / ( float )level;
		float b = 1.0f - a;

		bspVertex_t tmp[ 3 ];

		// Compute three verts for a triangle
		for ( int j = 0; j < 3; ++j )
		{
			int k = j * 3;
			tmp[ j ] = 
				*( controlPoints[ k + 0 ] ) * ( b * b ) + 
				*( controlPoints[ k + 1 ] ) * ( 2 * b * a ) +
				*( controlPoints[ k + 2 ] ) * ( a * a );
		}

		// Compute the inner layer of the bezier spline
		for ( int j = 0; j <= level; ++j )
		{
			float a1 = ( float )j / ( float )level;
			float b1 = 1.0f - a1;

			vertices[ i * L1 + j ] = 
				tmp[ 0 ] * ( b1 * b1 ) + 
				tmp[ 1 ] * ( 2 * b1 * a1 ) +
				tmp[ 2 ] * ( a1 * a1 );
		}
 	}

	// Compute the indices, which are designed to be used for a tri strip.
	indices.resize( level * L1 * 2 );

	for ( int row = 0; row < level; ++row )
	{
		for ( int col = 0; col <= level; ++col )
		{
			indices[ ( row * L1 + col ) * 2 + 0 ] = ( row + 1 ) * L1 + col;
			indices[ ( row * L1 + col ) * 2 + 1 ] = row * L1 + col;
		}
	}
}

void BezPatch::Render( void ) const
{
	GL_CHECK( glBindVertexArray( vao ) );

	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, buffers[ 0 ] ) );
	GL_CHECK( glBufferData( GL_ARRAY_BUFFER, sizeof( bspVertex_t ) * vertices.size(), &vertices[ 0 ], GL_DYNAMIC_DRAW ) );

	GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, buffers[ 1 ] ) );
	GL_CHECK( glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( GLuint ) * indices.size(), &indices[ 0 ], GL_DYNAMIC_DRAW ) ); 

	LoadVertexLayout();

	GL_CHECK( glDrawElements( GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, NULL ) );
}
