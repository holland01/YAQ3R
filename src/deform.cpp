#include "deform.h"
#include "log.h"
#include "q3bsp.h"
#include "glutil.h"
#include "effect_shader.h"

static void SubDivide_r( 
	deformModel_t& outModel, 
	const triangle_t& candidate, 
	const std::vector< bspVertex_t >& vertices, 
	const int level,
	const int max )
{
	if ( level == max )
		return;

	float maxLength = 0.0f;

	int oIndex = 0;
	int aIndex = 0;
	int bIndex = 0;

	glm::vec3 subend( 0.0f );

	glm::mat3 baryToWorld( 
		vertices[ candidate.indices[ 0 ] ].position, 
		vertices[ candidate.indices[ 1 ] ].position, 
		vertices[ candidate.indices[ 2 ] ].position );

	for ( int i = 0; i < 3; ++i )
	{
		int a = ( i + 1 ) % 3;
		int b = ( i + 2 ) % 3;

		glm::vec3 baryOpposite( 0.0f );
		baryOpposite[ a ] = 0.5f;
		baryOpposite[ b ] = 0.5f;

		glm::vec3 worldOpposite( baryToWorld * baryOpposite );

		float lineLength = glm::length( worldOpposite - vertices[ candidate.indices[ i ] ].position );

		if ( lineLength > maxLength )
		{
			subend = worldOpposite;

			oIndex = i;
			aIndex = a;
			bIndex = b;

			maxLength = lineLength;
		}
	}

	bspVertex_t newVertex = 
	{
		subend, 
		{ glm::vec2( 0.0f ), glm::vec2( 0.0f ) },
		glm::vec3( 0.0f ),

		{ 255, 255, 255, 255 }
	};

	std::vector< bspVertex_t > newBuffer = 
	{
		vertices[ candidate.indices[ 0 ] ], 
		vertices[ candidate.indices[ 1 ] ],
		vertices[ candidate.indices[ 2 ] ],
		newVertex
	};

	triangle_t t1 = 
	{
		{
			oIndex, 
			aIndex,
			3
		}
	};

	triangle_t t2 = 
	{
		{
			3,
			bIndex,
			oIndex
		}
	};

	SubDivide_r( outModel, t1, newBuffer, level + 1, max );
	SubDivide_r( outModel, t2, newBuffer, level + 1, max );

	if ( ( level + 1 ) == max )
	{
		int base = outModel.vertices.size();

		// If we're not empty, we need to sub by one to produce a zero-indexed offset
		if ( base )
			base -= 1;

		for ( int i = 0; i < 3; ++i )
		{
			t1.indices[ i ] += base;
			t2.indices[ i ] += base;
		}

		outModel.tris.push_back( t1 );
		outModel.tris.push_back( t2 );
		
		outModel.vertices.reserve( outModel.vertices.size() + newBuffer.size() );

		outModel.vertices.insert( outModel.vertices.end(), newBuffer.begin(), newBuffer.end() );
	}
}

void TessellateTri( std::vector< glm::vec3 >& outVerts, const float amount, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& surfaceNormal )
{
	
	const float step = 1.0f / amount;


	// this commented block isn't necessary at the moment, but it's kept here for reference just in case.
	// source: http://math.stackexchange.com/a/28552/17278
	/*
	const float triAreaFrac = 1.0f / ( glm::length( glm::cross( v1, v2 ) ) * 0.5f );

	// Convert a point P to barycentric coordinates, using the triangle's vertices.
	// If any of the barycentric coordinates (alpha, beta, gamma) are not in [0, 1], 
	// then the point lies outside the bounds of the triangle
	auto LPointInTriangle = [&a, &b, &c, &triAreaFrac]( const glm::vec3& p ) -> bool
	{
		glm::vec3 pb( b - p ), pa( a - p ), pc( c - p );

	 	const float alpha = glm::length( glm::cross( pb, pa ) ) * 0.5f * triAreaFrac;
		const float beta = glm::length( glm::cross( pb, pc ) ) * 0.5f * triAreaFrac;
		const float gamma = 1.0f - alpha - beta;
	
		return ( 0.0f <= alpha && alpha <= 1.0f ) && ( 0.0f <= beta && beta <= 1.0f ) && ( 0.0f <= gamma && gamma <= 1.0f ) && ( alpha + beta + gamma ) <= 1.0f;
	};
	*/

	const glm::vec3 a0( a * step );
	const glm::vec3 b0( b * step );
	const glm::vec3 c0( c * step );

	const float numAB( glm::floor( glm::length( b - a ) / glm::length( b0 - a0 ) ) );
	const float numBC( glm::floor( glm::length( c - b ) / glm::length( c0 - b0 ) ) );
	const float numCA( glm::floor( glm::length( a - c ) / glm::length( a0 - c0 ) ) );

	struct edgeData_t
	{
		float amount;
		glm::vec3 subedge;
		glm::vec3 start;
	} 
	edges[ 3 ] = 
	{
		{ numAB, b0 - a0, a },
		{ numBC, c0 - b0, b, },
		{ numCA, a0 - c0, c }
	};

	// TODO: now that the perimeter is properly taken care of, find a way to fill in the blank spots.
	// For a given outer row of triangles, the amount of upside down triangles (of the same dimensions)
	// which will fit in the open areas of the row is the amount of triangles in the row minus one.
	for ( int i = 0; i < 3; ++i )
	{
		for ( float walk = 0.0f; walk < edges[ i ].amount; walk += 1.0f )
		{
			glm::vec3 offset( edges[ i ].start + edges[ i ].subedge * walk );
			
			// We subtract by this for each point to accomodate for the amount which the origin of
			// the edge offsets each vertex of the triangle; without this, the first
			// triangle for a given edge iteration will reside outside of the triangle. The subtraction
			// ensures the vertices move in the opposite direction with respect to the edge origin
			glm::vec3 diff( edges[ i ].start * step );

			glm::vec3 v1( offset + a0 - diff );
			glm::vec3 v2( offset + b0 - diff );
			glm::vec3 v3( offset + c0 - diff );
			
			outVerts.push_back( v1 );
			outVerts.push_back( v2 );
			outVerts.push_back( v3 );
		}
	}
}

void Tessellate( deformModel_t* outModel, const mapData_t* data, const std::vector< GLuint >& indices, const bspVertex_t* vertices, float amount )
{
	assert( amount != 0 );

	const float step = 1.0f / amount;

	const bspFace_t* modelFace = data->faces + outModel->face;

	for ( uint32_t i = 0; i < indices.size(); i += 3 )
	{
		
	}
}

BezPatch::BezPatch( void )
	: lastCount( 0 )
{
	GL_CHECK( glGenBuffers( 1, &vbo ) ); 
	memset( controlPoints, 0, sizeof( const bspVertex_t* ) * BEZ_CONTROL_POINT_COUNT );  
}

BezPatch::~BezPatch( void )
{
	GL_CHECK( glDeleteBuffers( 1, &vbo ) );
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

	rowIndices.resize( level );
	trisPerRow.resize( level );
	for ( int row = 0; row < level; ++row )
	{
		trisPerRow[ row ] = 2 * L1;
		rowIndices[ row ] = &indices[ row * 2 * L1 ];  
	}

	subdivLevel = level;
}

void BezPatch::Render( void ) const
{
	LoadBufferLayout( vbo );

	if ( lastCount < vertices.size() )
		GL_CHECK( glBufferData( GL_ARRAY_BUFFER, sizeof( bspVertex_t ) * vertices.size(), &vertices[ 0 ], GL_DYNAMIC_DRAW ) );
	else
		GL_CHECK( glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof( bspVertex_t ) * vertices.size(), &vertices[ 0 ] ) );

	GL_CHECK( glMultiDrawElements( GL_TRIANGLE_STRIP, &trisPerRow[ 0 ], GL_UNSIGNED_INT, ( const GLvoid** ) &rowIndices[ 0 ], subdivLevel ) );

	lastCount = vertices.size();
}

