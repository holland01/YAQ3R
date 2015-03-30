#include "deform.h"
#include "log.h"
#include "q3bsp.h"
#include "glutil.h"
#include "effect_shader.h"
#include <cmath>
#include <memory>
#include <limits>

// Computations shamelessly stolen
// from http://gamedev.stackexchange.com/a/49370/8185
struct baryCoordSystem_t
{
	glm::vec3 a;

	glm::vec3 v0;
	glm::vec3 v1;
	
	float d00;
	float d01;
	float d11;

	float D;

	baryCoordSystem_t( const glm::vec3& va, const glm::vec3& vb, const glm::vec3& vc )
		: a( va ),
		  v0( vb - a ), v1( vc - a ),
		  d00( glm::dot( v0, v0 ) ),
		  d01( glm::dot( v0, v1 ) ),
		  d11( glm::dot( v1, v1 ) ),
		  D( 1.0f / ( d00 * d11 - d01 * d01 ) )
	{	
	}

	~baryCoordSystem_t( void )
	{
	}

	glm::vec3 ToBaryCoords( const glm::vec3& p ) const
	{
		glm::vec3 v2( p - a );
		
		float d20 = glm::dot( v2, v0 );
		float d21 = glm::dot( v2, v1 );

		glm::vec3 b;
		b.x = ( d11 * d20 - d01 * d21 ) * D;
		b.y = ( d00 * d21 - d01 * d20 ) * D;
		b.z = 1.0f - b.x - b.y;

		return b;
	}

	bool IsInTri( const glm::vec3& p ) const
	{
		glm::vec3 bp( ToBaryCoords( p ) );

		return ( 0.0f <= bp.x && bp.x <= 1.0f ) 
			&& ( 0.0f <= bp.y && bp.y <= 1.0f )
			&& ( 0.0f <= bp.z && bp.z <= 1.0f );
	}
};

//-------------------------------------------------

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
void BezPatch::Tessellate( int level )
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

//----------------------------------------------------------

void TessellateTri( 
	std::vector< bspVertex_t >& outVerts, 
	std::vector< triangle_t >& outIndices,
	const float amount, 
	const bspVertex_t& a, 
	const bspVertex_t& b, 
	const bspVertex_t& c
)
{
	auto LPassedC = [ &c ]( const glm::vec3& point, const glm::vec3& sig ) -> bool
	{
		return ( glm::sign( c.position - point ) != sig );
	};

	auto LSetTexCoords = []( bspVertex_t& vert, float x, float y )
	{
		vert.texCoords[ 0 ].x = vert.texCoords[ 1 ].x = x;
		vert.texCoords[ 0 ].y = vert.texCoords[ 1 ].y = y;
	};

	using tessLambda_t = std::function< void( const glm::vec3& a2, const glm::vec3& b2 ) >;

	float step = 1.0f / amount;

	// Use these as a basis for when either the a or b traversal vectors
	// have passed vertex c
	glm::vec3 aSig = glm::sign( c.position - a.position );
	glm::vec3 bSig = glm::sign( c.position - b.position ); 

	bool aPassed = false;
	bool bPassed = false;

	std::unique_ptr< baryCoordSystem_t > triCoordSys( new baryCoordSystem_t( a.position, b.position, c.position ) );

	//glm::vec3 bToC( ( c.position - b.position ) * step );
	//glm::vec3 aToC( ( c.position - a.position ) * step );

	bspVertex_t bToC( ( c - b ) * step );
	bspVertex_t aToC( ( c - a ) * step );
	bspVertex_t aToBStep( ( b - a ) * step );

	//glm::vec3 aToBStep( ( b.position - a.position )  * step );



	std::vector< triangle_t > curStrip;

	// Points which walk along the edges
	//glm::vec3 a2( a.position );
	//glm::vec3 b2( b.position );

	bspVertex_t a2( a );
	bspVertex_t b2( b );

	while ( true )
	{
		if ( glm::any( glm::isnan( a2.position ) ) || glm::any( glm::isnan( b2.position ) ) )
			break;

		// If either of these are set to true, then we'll have
		// triangles which exist outside of the parent tri.
		if ( LPassedC( a2.position, aSig ) || LPassedC( b2.position, bSig ) )
			break;

		if ( a2.position == c.position || b2.position == c.position )
			break;

		//glm::vec3 aToB( b2 - a2 );
		 
		// Path trace the edges of our triangle defined by vertices a2 and b2
		//glm::vec3 end( a2 + aToB );

		bspVertex_t aToB( b2 - a2 );
		bspVertex_t end( a2 + aToB );

		float walk = 0.0f;
		float walkLength = 0.0f;
		float walkStep = glm::length( aToBStep.position ) / glm::length( aToB.position );
		float endLength = glm::length( aToB.position );
		
		int colorCounter = 0;

		while ( walkLength < endLength )
		{
			//glm::vec3 v1( a2 + aToB * walk );
			//glm::vec3 v2( v1 + aToBStep );
			//glm::vec3 v3( v1 + aToC );

			bspVertex_t gv1( a2 + aToB * walk ); 
			bspVertex_t gv2( gv1 + aToBStep );
			bspVertex_t gv3( gv1 + aToC );
			bspVertex_t gv4( gv3 + aToBStep );
			glm::u8vec4 color;

			//gv1.position = v1;
			//gv2.position = v2;
			//gv3.position = v3;
			
			// There should be a reasonable workaround for this; maybe scale
			// the vertices or something like that.
			if ( !triCoordSys->IsInTri( gv3.position ) || !triCoordSys->IsInTri( gv2.position ) )
				goto end_iteration;

			size_t numVertices = outVerts.size();
				
			switch ( colorCounter++ % 3 )
			{
			case 0:
				color = glm::u8vec4( 0, 0, 0, 255 );  
				break;
			case 1:
				color = glm::u8vec4( 128, 128, 128, 255 );
				break;
			case 2:
				color = glm::u8vec4( 255, 255, 255, 255 );
				break;
			}

			gv1.color = color;
			gv2.color = color;
			gv3.color = color;

			triangle_t t1;

			{
				auto v1Iter = std::find( outVerts.begin(), outVerts.end(), gv1 );
				if ( v1Iter == outVerts.end() )
				{
					outVerts.push_back( gv1 );
					t1.indices[ 0 ] = numVertices++;
				}
				else
				{
					t1.indices[ 0 ] = v1Iter - outVerts.begin(); 
				}

				auto v2Iter = std::find( outVerts.begin(), outVerts.end(), gv2 );
				if ( v2Iter == outVerts.end() )
				{
					outVerts.push_back( gv2 );
					t1.indices[ 1 ] = numVertices++;
				}
				else
				{
					t1.indices[ 1 ] = v2Iter - outVerts.begin(); 
				}

				auto v3Iter = std::find( outVerts.begin(), outVerts.end(), gv3 );
				if ( v3Iter == outVerts.end() )
				{
					outVerts.push_back( gv3 );
					t1.indices[ 2 ] = numVertices++;
				}
				else
				{
					t1.indices[ 2 ] = v3Iter - outVerts.begin(); 
				}
			}

			curStrip.push_back( t1 );

			// Attempt a second triangle, providing v4
			// is within the bounds
			//glm::vec3 v4( v3 + aToBStep );
			
			if ( !triCoordSys->IsInTri( gv4.position ) )
				goto end_iteration;

			{
				//bspVertex_t gv4;
				//gv4.position = v4;
				auto v4Iter = std::find( outVerts.begin(), outVerts.end(), gv4 );

				triangle_t t2 = 
				{
					{
						t1.indices[ 2 ],
						t1.indices[ 1 ],
						0
					}
				};
			
				if ( v4Iter == outVerts.end() )
				{
					outVerts.push_back( gv4 );
					t2.indices[ 2 ] = numVertices;
				}
				else
				{
					t2.indices[ 2 ] = v4Iter - outVerts.begin();
				}

				curStrip.push_back( t2 );
			}	

end_iteration:
			walk += walkStep;
			walkLength = glm::length( aToB.position * walk );
		}
		
		outIndices.insert( outIndices.end(), curStrip.begin(), curStrip.end() );
		curStrip.clear();

		a2 += aToC;
		b2 += bToC;
	}
}

