#include "deform.h"
#include "renderer.h"
#include "io.h"
#include "q3bsp.h"
#include "glutil.h"
#include "effect_shader.h"
#include <cmath>
#include <memory>
#include <limits>
#include <algorithm>

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

// Algorithm behind table generation is taken from here: https://github.com/id-Software/Quake-III-Arena/blob/master/code/renderer/tr_init.c#L1042
// 
// On the triangle wave:
// The first half of the table is laid out in the following format:
// [0.0, 0.01, 0.02, ..., 0.8, 0.9, 1.0, 1.0, 0.9, 0.8, ..., 0.02, 0.01, 0.0]
// where all of the numbers are in the range [0, 1], and distributed as a segment of 256.
// The second half is literally just the negatives of these same values.
deformGlobal_t deformCache = 
{ 
	// sine wave table
	[]( void )-> std::array< float, DEFORM_TABLE_SIZE >
	{
		std::array< float, DEFORM_TABLE_SIZE > ret;

		for ( int i = 0; i < DEFORM_TABLE_SIZE; ++i )
		{
			ret[ i ] = glm::sin( ( float ) i / DEFORM_TABLE_SIZE );
		}

		return ret;
	}(),

	// triangle wave table
	[]( void )-> std::array< float, DEFORM_TABLE_SIZE > 
	{
		std::array< float, DEFORM_TABLE_SIZE > ret;

		for ( int i = 0; i < DEFORM_TABLE_SIZE; ++i )
		{
			if ( i < DEFORM_TABLE_SIZE / 2 )
			{
				if ( i < DEFORM_TABLE_SIZE / 4 )
				{
					ret[ i ] = ( float ) i / ( DEFORM_TABLE_SIZE / 4 );
				}
				else
				{
					ret[ i ] = 1.0f - ret[ i - DEFORM_TABLE_SIZE / 4 ];
				}
			}
			else
			{
				ret[ i ] = -ret[ i - DEFORM_TABLE_SIZE / 2 ];
			}
		}

		return ret;
	}() 
};

float GenDeformScale( const glm::vec3& position, const shaderInfo_t* shader )
{
	// The solution here is also snagged from the Q3 engine.
	if ( !shader )
		return 0.0f;

	switch ( shader->deformCmd )
	{
	case VERTEXDEFORM_CMD_WAVE:
		{
			switch ( shader->deformFn )
			{
			case VERTEXDEFORM_FUNC_TRIANGLE:
				{
					// Distribute the "weight" of the tessellation spread across the length of the vertex position vector, where the vertex's tail is located at the world origin.
                    float offset =
                            shader->deformParms.data.wave.phase + ( position.x + position.y + position.z ) * shader->deformParms.data.wave.spread;

                    return DEFORM_CALC_TABLE( deformCache.triTable, shader->deformParms.data.wave.base, offset, glfwGetTime(), shader->deformParms.data.wave.frequency,
                        shader->deformParms.data.wave.amplitude );
				}
				break;
            default:
                break;
			}
		}
		break;

	default:
		MLOG_WARNING( "Non-implemented vertex deform command specified." );
		break;
	}

	return 0.0f;
}

//----------------------------------------------------------

void GenPatch( mapModel_t* model, const shaderInfo_t* shader, int controlPointStart, int indexOffset )
{
	if ( !model->subdivLevel )
	{
		if ( shader && shader->tessSize != 0.0f )
			model->subdivLevel = ( int )shader->tessSize;
		else
			model->subdivLevel = 10;
	}

    const size_t vertexStart = model->patchVertices.size();

	// Vertex count along a side is 1 + number of edges
    const int L1 = model->subdivLevel + 1;
    model->patchVertices.resize( vertexStart + L1 * L1 );

	// Compute the first spline along the edge
	for ( int i = 0; i <= model->subdivLevel; ++i )
	{
		float a = ( float )i / ( float )model->subdivLevel;
		float b = 1.0f - a;

        model->patchVertices[ vertexStart + i ] =
			*( model->controlPoints[ controlPointStart + 0 ] ) * ( b * b ) +
		 	*( model->controlPoints[ controlPointStart + 3 ] ) * ( 2 * b * a ) + 
			*( model->controlPoints[ controlPointStart + 6 ] ) * ( a * a );
	}

	// Go deep and fill in the gaps; outer loop is the first layer of curves
	for ( int i = 1; i <= model->subdivLevel; ++i )
	{
		float a = ( float )i / ( float )model->subdivLevel;
		float b = 1.0f - a;

		bspVertex_t tmp[ 3 ];

		// Compute three verts for a triangle
		for ( int j = 0; j < 3; ++j )
		{
			int k = j * 3;
			tmp[ j ] = 
				*( model->controlPoints[ controlPointStart + k + 0 ] ) * ( b * b ) + 
				*( model->controlPoints[ controlPointStart + k + 1 ] ) * ( 2 * b * a ) +
				*( model->controlPoints[ controlPointStart + k + 2 ] ) * ( a * a );
		}

		// Compute the inner layer of the bezier spline
		for ( int j = 0; j <= model->subdivLevel; ++j )
		{
			float a1 = ( float )j / ( float )model->subdivLevel;
			float b1 = 1.0f - a1;

            bspVertex_t& v = model->patchVertices[ vertexStart + i * L1 + j ];

			v = tmp[ 0 ] * ( b1 * b1 ) + 
				tmp[ 1 ] * ( 2 * b1 * a1 ) +
				tmp[ 2 ] * ( a1 * a1 );
		}
 	}

	// Compute the indices, which are designed to be used for a tri strip.
	const size_t indexStart = model->indices.size();
	model->indices.resize( indexStart + model->subdivLevel * L1 * 2 );

	for ( int row = 0; row < model->subdivLevel; ++row )
	{
		for ( int col = 0; col <= model->subdivLevel; ++col )
		{
			model->indices[ indexStart + ( row * L1 + col ) * 2 + 0 ] = indexOffset + vertexStart + ( row + 1 ) * L1 + col;
			model->indices[ indexStart + ( row * L1 + col ) * 2 + 1 ] = indexOffset + vertexStart + row * L1 + col;
		}
	}
}

//----------------------------------------------------------

void TessellateTri( 
	std::vector< bspVertex_t >& outVerts, 
	std::vector< triangle_t >& outIndices,
	float amount,
	float normalOffsetScale,
	const bspVertex_t& a, 
	const bspVertex_t& b, 
	const bspVertex_t& c
)
{
	auto LPassedC = [ &c ]( const glm::vec3& point, const glm::vec3& sig ) -> bool
	{
		return ( glm::sign( c.position - point ) != sig );
	};

	float step = 1.0f / amount;

	// Use these as a basis for when either the a or b traversal vectors
	// have passed vertex c
	glm::vec3 aSig = glm::sign( c.position - a.position );
	glm::vec3 bSig = glm::sign( c.position - b.position ); 

	std::unique_ptr< baryCoordSystem_t > triCoordSys( new baryCoordSystem_t( a.position, b.position, c.position ) );

	bspVertex_t bToC( ( c - b ) * step );
	bspVertex_t aToC( ( c - a ) * step );
	bspVertex_t aToBStep( ( b - a ) * step );

	std::vector< triangle_t > curStrip;

	// Points which walk along the edges
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
		 
		// Path trace the edges of our triangle defined by vertices a2 and b2
		bspVertex_t aToB( b2 - a2 );

		float walk = 0.0f;
		float walkLength = 0.0f;
		float walkStep = glm::length( aToBStep.position ) / glm::length( aToB.position );
		float endLength = glm::length( aToB.position );

		while ( walkLength < endLength )
		{
			bspVertex_t gv1( a2 + aToB * walk ); 
			bspVertex_t gv2( gv1 + aToBStep );
			bspVertex_t gv3( gv1 + aToC );
			bspVertex_t gv4( gv3 + aToBStep );

			gv1.position += gv1.normal * normalOffsetScale;
			gv2.position += gv2.normal * normalOffsetScale;
			gv3.position += gv3.normal * normalOffsetScale;
			gv4.position += gv4.normal * normalOffsetScale;

            size_t numVertices;
            triangle_t t1;
			
			// There should be a reasonable workaround for this; maybe scale
			// the vertices or something like that.
			if ( !triCoordSys->IsInTri( gv3.position ) || !triCoordSys->IsInTri( gv2.position ) )
				goto end_iteration;

            numVertices = outVerts.size();

			gv1.color = glm::u8vec4( 255 );
			gv2.color = glm::u8vec4( 255 );
			gv3.color = glm::u8vec4( 255 );

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
			if ( !triCoordSys->IsInTri( gv4.position ) )
				goto end_iteration;

			{
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

