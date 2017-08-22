#include "deform.h"
#include "renderer.h"
#include "model.h"
#include "io.h"
#include "q3bsp.h"
#include "glutil.h"
#include "effect_shader.h"
#include "lib/random.h"
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
deformGlobal_t gDeformCache =
{
	nullptr,// skyShader

	0,		// skyVbo
	0,		// skyIbo

	0,		// numSkyIndices
	0,		// numSkyVertices

	0.0f,	// skyHeightOffset
	1.0f,	// waterFormScalar

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
	}(),
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
					// Distribute the "weight" of the tessellation spread
					// across the length of the vertex position vector,
					// where the vertex's tail is located at the world origin.
					float offset =
							shader->deformParms.data.wave.phase
							+ ( position.x + position.y + position.z )
							* shader->deformParms.data.wave.spread;

					float t = GetTimeSeconds();

					return DEFORM_CALC_TABLE(
						gDeformCache.triTable,
						shader->deformParms.data.wave.base,
						offset,
						t,
						shader->deformParms.data.wave.frequency,
						shader->deformParms.data.wave.amplitude
					);
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

deformGlobal_t::~deformGlobal_t( void )
{
	DeleteBufferObject( GL_ARRAY_BUFFER, skyVbo );
	DeleteBufferObject( GL_ELEMENT_ARRAY_BUFFER, skyIbo );
}

//----------------------------------------------------------
// InitSkyData
//
// Nothing major.
void deformGlobal_t::InitSkyData( float cloudHeight )
{
	float radius = 4096.0f;

	float subdivCount = 16.0f;

	float thetaStep = glm::two_pi< float >() / subdivCount;
	float phiStep = glm::pi< float >() / subdivCount;

	float toS = 1.0f / glm::two_pi< float >();
	float toT = 1.0f / glm::pi< float >();

	GLushort maxValue = 0;
	GLushort maxRow = 0;
	GLushort maxCol = 0;

	auto LIndexFromAngles = [ &maxRow, &maxCol, &maxValue, &toS, &toT, &subdivCount ]( float theta, float phi ) -> GLushort
	{
		GLushort col = static_cast< GLushort >( theta * toS * subdivCount ); 	// from [0, 2pi] to [0, subdivCount]
		GLushort row = static_cast< GLushort >( phi * toT * subdivCount );		// from [0, pi] to [0, subdivCount]

		GLushort ret = row * subdivCount + col;

		if ( ret > maxValue )
		{
			maxValue = ret;
			maxRow = row;
			maxCol = col;
		}

		return ret;
	};

	// Note: Theta moves clockwise instead of counter-clockwise.
	auto LVertexFromAngles = [ this, &radius, &cloudHeight, &toS, &toT ]( float theta, float phi ) -> bspVertex_t
	{
		glm::vec2 tex {
			glm::cos( theta ) * glm::cos( phi ) * 0.5f + 0.5f,
			glm::sin( theta ) * glm::cos( phi ) * 0.5f + 0.5f
		};

		tex.s = glm::max( 0.0f, ( ( tex.s * 256.0f ) - 1.1f ) / 256.0f );
		tex.t = glm::max( 0.0f, ( ( tex.t * 256.0f ) - 1.1f ) / 256.0f );

		return {
			{
				radius * glm::cos( theta ) * glm::cos( phi ),
				skyHeightOffset + cloudHeight * glm::sin( phi ),
				radius * glm::sin( theta ) * glm::cos( phi )
			},
			{
				tex,
				tex
			},
			{
				0.0f, 0.0f, 0.0f
			},
			{
				255, 255, 255, 255
			}
		};
	};

	std::vector< bspVertex_t > skyVerts;
	std::vector< GLushort > skyIndices;

	skyVerts.reserve( subdivCount * subdivCount );
	skyIndices.reserve( subdivCount * subdivCount * 6 );

	for ( float phi = 0.0f; phi <= glm::pi< float >(); phi += phiStep )
	{
		for ( float theta = 0.0f; theta <= glm::two_pi< float >(); theta += thetaStep  )
		{
			skyVerts.push_back( LVertexFromAngles( theta, phi ) );

			skyIndices.push_back( LIndexFromAngles( theta, phi + phiStep ) );
			skyIndices.push_back( LIndexFromAngles( theta + thetaStep, phi + phiStep ) );
			skyIndices.push_back( LIndexFromAngles( theta, phi ) );

			skyIndices.push_back( LIndexFromAngles( theta + thetaStep, phi ) );
			skyIndices.push_back( LIndexFromAngles( theta + thetaStep, phi + phiStep ) );
			skyIndices.push_back( LIndexFromAngles( theta, phi ) );
		}
	}

	skyVbo = GenBufferObject< bspVertex_t >( GL_ARRAY_BUFFER, skyVerts, GL_STATIC_DRAW );
	skyIbo = GenBufferObject< GLushort >( GL_ELEMENT_ARRAY_BUFFER, skyIndices, GL_STATIC_DRAW );

	numSkyIndices = static_cast< GLsizei >( skyIndices.size() );
	numSkyVertices = static_cast< GLsizei >( skyVerts.size() );

	MLOG_INFO_ONCE(
		"skyVbo: %u, skyIbo: %u, cloudHeight: %f, numSkyVertices: %li, numSkyIndices: %li, maxIndex: %u\n"
		"maxRow: %u, maxCol: %u",
		skyVbo, skyIbo, cloudHeight, numSkyVertices, numSkyIndices, maxValue, maxRow, maxCol );
}

//----------------------------------------------------------
// Adapted from http://graphics.cs.brown.edu/games/quake/quake3.html
// In particular, the "Rendering Faces" section.
void GenPatch(
	gIndexBuffer_t& outIndices,
	mapPatch_t* model,
	const shaderInfo_t* shader,
	int controlPointStart,
	int indexOffset
)
{
	if ( !model->subdivLevel )
	{
		if ( shader && shader->tessSize != 0.0f )
			model->subdivLevel = ( int )shader->tessSize;
		else
			model->subdivLevel = 5;
	}

	const size_t vertexStart = model->clientVertices.size();

	// Vertex count along a side is 1 + number of edges
	const int L1 = model->subdivLevel + 1;
	model->clientVertices.resize( vertexStart + L1 * L1 );

	// Compute the first spline along the edge
	for ( int i = 0; i < L1; ++i )
	{
		float a = ( float )i / ( float )model->subdivLevel;
		float b = 1.0f - a;

		model->clientVertices[ vertexStart + i ] =
			*( model->controlPoints[ controlPointStart + 0 ] ) * ( b * b ) +
			*( model->controlPoints[ controlPointStart + 3 ] ) * ( 2 * b * a ) +
			*( model->controlPoints[ controlPointStart + 6 ] ) * ( a * a );
	}

	// Go deep and fill in the gaps; outer loop is the first layer of curves
	for ( int i = 1; i < L1; ++i )
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
		for ( int j = 0; j < L1; ++j )
		{
			float a1 = ( float )j / ( float )model->subdivLevel;
			float b1 = 1.0f - a1;

			bspVertex_t& v = model->clientVertices[ vertexStart + i * L1 + j ];

			v = tmp[ 0 ] * ( b1 * b1 ) +
				tmp[ 1 ] * ( 2 * b1 * a1 ) +
				tmp[ 2 ] * ( a1 * a1 );
		}
	}

	// Compute the indices, which are designed to be used for a tri strip.
	const size_t indexStart = outIndices.size();
	outIndices.resize( indexStart + model->subdivLevel * L1 * 2 );

	for ( int y = 0; y < model->subdivLevel; ++y )
	{
		for ( int x = 0; x < L1; ++x )
		{
			outIndices[ indexStart + ( y * L1 + x ) * 2 + 0 ] =
				indexOffset + vertexStart + ( y + 1 ) * L1 + x;
			outIndices[ indexStart + ( y * L1 + x ) * 2 + 1 ] =
				indexOffset + vertexStart + ( y + 0 ) * L1 + x;
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
	auto LPassedC = [ &c ]( const glm::vec3& point,
		const glm::vec3& sig ) -> bool
	{
		return ( glm::sign( c.position - point ) != sig );
	};

	float step = 1.0f / amount;

	// Use these as a basis for when either the a or b traversal vectors
	// have passed vertex c
	glm::vec3 aSig = glm::sign( c.position - a.position );
	glm::vec3 bSig = glm::sign( c.position - b.position );

	std::unique_ptr< baryCoordSystem_t > triCoordSys(
		new baryCoordSystem_t( a.position, b.position, c.position ) );

	bspVertex_t bToC( ( c - b ) * step );
	bspVertex_t aToC( ( c - a ) * step );
	bspVertex_t aToBStep( ( b - a ) * step );

	std::vector< triangle_t > curStrip;

	// Points which walk along the edges
	bspVertex_t a2( a );
	bspVertex_t b2( b );

	while ( true )
	{
		if ( glm::any( glm::isnan( a2.position ) )
		|| glm::any( glm::isnan( b2.position ) ) )
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
		float walkStep = glm::length( aToBStep.position )
			/ glm::length( aToB.position );
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
			if ( !triCoordSys->IsInTri( gv3.position )
			|| !triCoordSys->IsInTri( gv2.position ) )
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

				auto v2Iter = std::find( outVerts.begin(), outVerts.end(),
					gv2 );
				if ( v2Iter == outVerts.end() )
				{
					outVerts.push_back( gv2 );
					t1.indices[ 1 ] = numVertices++;
				}
				else
				{
					t1.indices[ 1 ] = v2Iter - outVerts.begin();
				}

				auto v3Iter = std::find( outVerts.begin(), outVerts.end(),
					gv3 );
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
				auto v4Iter = std::find( outVerts.begin(), outVerts.end(),
					gv4 );

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
