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

	// Find matching winding order...
	glm::vec3 e1( a - b );
	glm::vec3 e2( c - a );

	glm::vec3 norm( glm::cross( e1, e2 ) );

	// Face opposing directions; so we reverse the ordering of the edges and recompute the cross
	if ( glm::dot( norm, surfaceNormal ) < 0 )
	{
		e1 = a - c;
		e2 = b - a;

		norm = glm::cross( e1, e2 );
		assert( glm::dot( norm, surfaceNormal ) > 0 );
	}

	// Use two of the edges to walk the triangle.
	// We also pick an origin vertex with which we can use to
	// offset our triangle walk relative to.
	glm::vec3 v1 = e1;
	glm::vec3 v2, v3, origin, origin3;
		
	// a is our tipping point; when v3 == v1 in our edge walk, we know 
	// we're finished
	if ( v1 == a - b )
	{
		v2 = c - b;
		v3 = c - a;
		origin = b;
		origin3 = a;
	}
	else
	{
		v2 = b - c;
		v3 = b - a;
		origin = c;
		origin3 = a; 
	}

	// TODO: to find if the generated vertices lie outside of the triangle, convert
	// the worldspace coordinates to barycentric coordinates and test for values 
	// which are > 1 or < 0.

	for ( float walk1 = 0.0f; walk1 <= 1.0f; walk1 += step )
	{
		glm::vec3 offset1 = v1 * walk1;
		
		for ( float walk2 = 0.0f; walk2 <= 1.0f; walk2 += step )
		{
			glm::vec3 offset2 = v2 * walk2;
			glm::vec3 offset3 = v3 * walk2;

			//glm::vec3 baseLine = ( origin3 + offset3 ) - ( origin + offset2 );
		//	float baseLineLen = glm::length( baseLine );

			//if ( baseLineLen < 1.0f )
				//break;

			glm::vec3 left = origin + offset1 + offset2;
			glm::vec3 right = origin + offset1 + offset2 + ( v2 * step );
			glm::vec3 up = origin + offset1 + offset2 + ( v2 * step * 0.5f ) + ( v1 * step );

			outVerts.push_back( left );
			outVerts.push_back( right );
			outVerts.push_back( up );
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
