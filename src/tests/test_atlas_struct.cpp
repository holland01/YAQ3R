#include "test_atlas_struct.h"
#include "lib/random.h"
#include "lib/math.h"
#include "renderer/util.h"
#include "renderer/buffer.h"
#include "io.h"
#include <algorithm>

namespace {

INLINE pointSet_t::iterator& Increment( pointSet_t::iterator& p, int x )
{
	for ( int i = 0; i < x; ++i )
		++p;

	return p;
}

Random< uint8_t > gColor( 128, 255 );

std::vector< glm::u8vec4 > gTable = { glm::u8vec4( 255, 0, 0, 255 ) }; // we start off with red, because it's reserved for the color of the grid

INLINE glm::u8vec4 UniqueColor( void )
{
	while ( true )
	{
		glm::u8vec4 c( gColor(), gColor(), gColor(), 255 );

		bool found = false;

		for ( const glm::u8vec4& color: gTable )
		{
			if ( c == color )
			{
				found = true;
				break;
			}
		}

		if ( !found )
		{
			gTable.push_back( c );
			return c;
		}
	}

	return glm::u8vec4( 0 ); // <___<
}

bool Colliding( const bounds_t& a, const bounds_t& b )
{
	std::array< glm::vec2, 4 > aPoints, bPoints;

	auto LGetPoints = []( const bounds_t& bounds, std::array< glm::vec2, 4 >& points )
	{
		glm::vec2 ex( bounds.dimX * 0.5f, 0.0f );
		glm::vec2 ey( bounds.dimY * 0.5f, 0.0f );

		points[ 0 ] = bounds.origin - ex - ey;
		points[ 1 ] = bounds.origin - ex + ey;
		points[ 2 ] = bounds.origin + ex + ey;
		points[ 3 ] = bounds.origin + ex - ey;
	};

	LGetPoints( a, aPoints );
	LGetPoints( b, bPoints );

	// Project A's right onto B's left
	if ( bPoints[ 0 ].y > aPoints[ 1 ].y )
	{
		return false;
	}

	if ( bPoints[ 1 ].y < aPoints[ 0 ].y )
	{
		return false;
	}

	// Project A's bottom onto B's top
	if ( bPoints[ 3 ].x < aPoints[ 0 ].x )
	{
		return false;
	}

	if ( bPoints[ 0 ].x > aPoints[ 3 ].x )
	{
		return false;
	}

	return true;
}

struct resolveParams_t
{
	float outer;
	float inner;
	glm::vec2 dir;
};

std::vector< int8_t > gMerged;

void ResolveCollision( std::vector< bounds_t >& boundsList,
					   uint16_t stagnant,
					   uint16_t moving,
					   uint16_t slotDims )
{
	float srcArea = boundsList[ stagnant ].dimX * boundsList[ stagnant ].dimY;
	float nextArea = boundsList[ moving ].dimX * boundsList[ moving ].dimY;

	uint32_t largest = srcArea > nextArea? stagnant: moving;

	// Find the direction with the shortest distance possible, which is needed to
	// resolve the collision between the two bounds
	std::array< resolveParams_t, 4 > resolveParams;

	glm::vec2 p( boundsList[ largest ].dimX * 0.5f, 0.0f );
	glm::vec2 q( 0.0f, boundsList[ largest ].dimY * 0.5f );
	glm::vec2 u( 1.0f, 0.0f );
	glm::vec2 v( 0.0f, 1.0f );
	glm::vec2 o( boundsList[ moving ].origin );

	float offset;
	glm::vec2 s;
	glm::vec2 s0;

	offset = slotDims - boundsList[ largest ].dimX;
	s = o - p;
	s0 = s - u * offset;
	resolveParams[ 0 ].outer = glm::distance( s0, s );
	resolveParams[ 0 ].inner = glm::distance( o - p, o );
	resolveParams[ 0 ].dir = -p;

	offset = slotDims - boundsList[ largest ].dimY;
	s = o + q;
	s0 = s + v * offset;
	resolveParams[ 1 ].outer = glm::distance( s0, s );
	resolveParams[ 1 ].inner = glm::distance( o + q, o );
	resolveParams[ 1 ].dir = q;

	offset = slotDims - boundsList[ largest ].dimX;
	s = o + p;
	s0 = s + u * offset;
	resolveParams[ 2 ].outer = glm::distance( s0, s );
	resolveParams[ 2 ].inner = glm::distance( o + p, o );
	resolveParams[ 2 ].dir = p;

	offset = slotDims - boundsList[ largest ].dimY;
	s = o - q;
	s0 = s - v * offset;
	resolveParams[ 3 ].outer = glm::distance( s0, s );
	resolveParams[ 3 ].inner = glm::distance( o - q, o );
	resolveParams[ 3 ].dir = -q;

	float minDistance = std::numeric_limits< float >::max();
	uint32_t d = 0;
	for ( uint32_t k = 0; k < resolveParams.size(); ++k )
	{
		const resolveParams_t& rp = resolveParams[ k ];
		if ( rp.outer < minDistance )
		{
			if ( boundsList[ moving ].dimX <= minDistance && boundsList[ moving ].dimY <= minDistance )
			{
				d = k;
				minDistance = rp.outer;
			}
		}
	}

	if ( minDistance == std::numeric_limits< float >::max() )
	{
		return;
	}

	while ( Colliding( boundsList[ moving ], boundsList[ stagnant ] ) )
	{
		boundsList[ moving ].origin += resolveParams[ d ].dir * 0.1f;
	}

	gMerged[ moving ] = true;
}

void FindMerge( std::vector< bounds_t >& boundsList, uint16_t src, uint16_t slotDims )
{
	gMerged[ src ] = true;
	float srcArea = boundsList[ src ].dimX * boundsList[ src ].dimY;
	float maxArea = slotDims * slotDims;

	uint16_t i;
	for ( i = 0; i < boundsList.size(); ++i )
	{
		if ( gMerged[ i ] )
		{
			continue;
		}

		float nextArea = boundsList[ i ].dimX * boundsList[ i ].dimY;

		if ( srcArea + nextArea < maxArea )
		{
			boundsList[ i ].origin = boundsList[ 0 ].origin;
			break;
		}
	}

	ResolveCollision( boundsList, src, i, slotDims );
}

uint16_t NextUnmerged( uint16_t start = 0 )
{
	uint16_t c = start;
	while ( c < gMerged.size() && gMerged[ c ] )
	{
		c++;
	}
	return c;
}

}

TAtlas::TAtlas( void )
	: Test( 800, 600, false ),
	  camera( new InputCamera() )
{
	context = SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
	camPtr = camera;
}

void TAtlas::Load( void )
{
	if ( !Test::Load( "woo" ) )
	{
		return;
	}

	Random< uint16_t > maxDimsGen( 256, 512 );
	uint16_t slotDims = NextPower2( maxDimsGen() );
	Random< uint8_t > numImages( 20, 50 );
	uint16_t nImage = numImages();
	uint16_t square = NextSquare( nImage );

	Random< uint16_t > dimGen( 32, slotDims );

	for ( uint16_t i = 0; i < square * square; ++i )
	{
		bounds_t box;

		box.color = UniqueColor();
		box.dimX = ( float ) NextPower2( dimGen() );
		box.dimY = ( float ) NextPower2( dimGen() );

		boundsList.push_back( box );
	}

	std::sort( boundsList.begin(), boundsList.end(), [ & ]( const bounds_t& a, const bounds_t& b ) -> bool
	{
		float al = glm::length( glm::vec2( a.dimX, a.dimY ) );
		float bl = glm::length( glm::vec2( b.dimX, b.dimY ) );

		return al < bl;
	} );

	float fSlotDims = ( float ) slotDims;

	for ( uint32_t y = 0; y < square; ++y )
	{
		for ( uint32_t x = 0; x < square; ++x )
		{
			boundsList[ y * square + x ].origin = glm::vec2( ( float ) x * fSlotDims, ( float ) y * fSlotDims );
		}
	}

	gMerged.resize( boundsList.size(), false );

	FindMerge( boundsList, 0, slotDims );
	FindMerge( boundsList, NextUnmerged(), slotDims );

	camPtr->SetPerspective( 90.0f, 800.0f, 600.0f, 1.0f, 5000.0f );

	GEnableDepthBuffer();

	GL_CHECK( glClearColor( 0.0f, 0.0f, 0.0f, 1.0f ) );
	GL_CHECK( glPointSize( 10 ) );

	camPtr->moveStep = 0.1f;

	MLOG_INFO( "Num Images: %i\n Square: %i\n Slot Dimensions: %i\n", nImage, square, slotDims );
}

void TAtlas::Run( void )
{
	const float Z_PLANE = -5.0f;

	camPtr->Update();

	std::for_each( boundsList.begin(), boundsList.end(), [ & ]( const bounds_t& box )
	{
		glm::mat4 v( camPtr->ViewData().transform );

		glm::vec3 s( box.origin, Z_PLANE );
		glm::vec3 e0( box.dimX * 0.5f, 0.0f, 0.0f );
		glm::vec3 e1( 0.0f, box.dimY * 0.5f, 0.0f );

		GU_ImmBegin( GL_LINE_LOOP, v, camPtr->ViewData().clipTransform );

		guImmPosList_t p =
		{
			s - e0 - e1,
			s - e0 + e1,
			s + e0 + e1,
			s + e0 - e1
		};
		GU_ImmLoad( p, box.color );
		GU_ImmEnd();
	} );

}

