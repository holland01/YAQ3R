#include "test_atlas_struct.h"
#include "lib/random.h"
#include "lib/math.h"
#include "renderer/util.h"
#include "renderer/buffer.h"
#include "io.h"
#include <algorithm>
#include <unordered_map>

struct bucket_t
{
	uint16_t count = 0;
	uint16_t val = 0;
	std::unique_ptr< bucket_t > next;

	void WriteCount( uint16_t c )
	{
		count = c | ( c << 8 );
	}

	void IncCount( void )
	{
		uint16_t c = count & 0xFF;
		c++;
		WriteCount( c );
	}

	uint8_t ReadCount( void )
	{
		return ( uint8_t )( c & 0xFF );
	}

	void SubOffset( void )
	{
		uint16_t off = ( c >> 8 ) & 0xFF;
		off--;
		c = ( c & 0xFF ) | ( off << 8 );
	}

	uint8_t ReadOffset( void )
	{
		return ( uint8_t ) ( ( c >> 8 ) & 0xFF );
	}


};

struct tree_t
{
	uint16_t key;
	std::unique_ptr< bucket_t > first;
	std::unique_ptr< tree_t > left;
	std::unique_ptr< tree_t > right;
};

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

void ShiftForward( std::unique_ptr< bucket_t >& newb, bucket_t* p, uint16_t v )
{
	newb->val = p->val;
	newb->count = p->count;
	newb->next = std::move( p->next );
	p->next = std::move( newb );
	p->val = v;
	p->WriteCount( 1 );
}

// Insert height values in descending order
void BucketInsert( tree_t& t, uint16_t v )
{
	if ( !t.first )
	{
		t.first.reset( new bucket_t() );
		t.first->val = v;
		t.first->count = 1;
		return;
	}

	bucket_t* prev, *p;

	prev = nullptr;
	p = t.first.get();

	while ( p && v < p->val )
	{
		prev = p;
		p = p->next.get();
	}

	if ( p && p->val == v )
	{
		p->count++;
		return;
	}

	if ( prev )
	{
		std::unique_ptr< bucket_t > newb( new bucket_t() );

		// p->val < v < prev->val
		if ( p )
		{
			ShiftForward( newb, p, v );
		}
		else
		{
			newb->val = v;
			newb->count = 1;
			prev->next = std::move( newb );
		}
	}
	// if prev == nullptr, p automatically isn't null
	// and v > first element, so we just correct that.
	else
	{
		std::unique_ptr< bucket_t > newb( new bucket_t() );
		ShiftForward( newb, p, v );
	}
}

std::unique_ptr< tree_t > TreeMake( uint16_t k, uint16_t v )
{
	std::unique_ptr< tree_t > t( new tree_t() );
	t->key = k;
	t->first.reset( new bucket_t() );
	t->first->val = v;
	return t;
}

void TreeInsert( tree_t& t, uint16_t k, uint16_t v );

void InsertOrMake( std::unique_ptr< tree_t >& t, uint16_t k, uint16_t v )
{
	if ( t )
	{
		TreeInsert( *t, k, v );
	}
	else
	{
		t = TreeMake( k, v );
	}
}

void TreeInsert( tree_t& t, uint16_t k, uint16_t v )
{
	if ( k < t.key )
	{
		InsertOrMake( t.left, k, v );
	}
	else if ( k > t.key )
	{
		InsertOrMake( t.right, k, v );
	}
	else
	{
		BucketInsert( t, v );
	}
}


// !FIXMEs:
// - occasional overlap on bottom height with other heights (by removing the count/node removal, this may be fixed now)
// - overlap with 128 width and 64 width
// - gap between 128/256; 128 should be placed in this gap.

// TODO:
// offset: the actual offset which decrements for duplicate height values
// count: the number of duplicate height values for a given bucket - this must
// stay the same, so that buckets of heights which are less than the current height's
// count will be able to take that value into account

void TreePoint( tree_t& t, bounds_t& bounds, int8_t sign )
{
	if ( bounds.dimX < t.key )
	{
		bounds.origin.x -= t.left->key;
		TreePoint( *( t.left ), bounds, 1 );
	}
	else if ( bounds.dimX > t.key )
	{
		bounds.origin.x += t.key;
		TreePoint( *( t.right ), bounds, 1 );
	}
	else
	{
		bucket_t * curr = t.first.get();

		// offset our origin by any heights which
		// are meant to occur before it - we also
		// take into account the amount of duplications
		// for every height value
		while ( curr && bounds.dimY < curr->val )
		{
			bounds.origin.y += ( float )curr->val * ( float )curr->count;
			curr = curr->next.get();
		}

		// if curr != nullptr, we know that, by nature of the this structure,
		// curr->val is implicitly == bounds.dimY
		if ( curr && curr->count > 1 )
		{
			bounds.origin.y += ( float )curr->val * ( float )( curr->count - 1 );

			curr->count--;
		}

		bounds.origin.x += sign * bounds.dimX * 0.5f;
		bounds.origin.y += bounds.dimY * 0.5f;
	}
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
	uint16_t maxImage = NextPower2( maxDimsGen() );
	uint16_t nImage = 30;

	Random< uint16_t > dimGen( 32, maxImage );

	glm::ivec2 totalRes;

	median_t widths, heights;

	for ( uint16_t i = 0; i < nImage; ++i )
	{
		bounds_t box;

		box.color = UniqueColor();
		box.dimX = NextPower2( dimGen() );
		box.dimY = NextPower2( dimGen() );

		totalRes.x += box.dimX;
		totalRes.y += box.dimY;

		widths.Insert( box.dimX );
		heights.Insert( box.dimY );

		boundsList.push_back( box );
	}

	tree.reset( new tree_t() );
	tree->key = widths.GetMedian();

	for ( uint16_t i = 0; i < boundsList.size(); ++i )
	{
		TreeInsert( *tree, boundsList[ i ].dimX, boundsList[ i ].dimY );
	}

	for ( uint32_t i = 0; i < boundsList.size(); ++i )
	{
		TreePoint( *tree, boundsList[ i ], 1 );
	}

	camPtr->SetPerspective( 90.0f, 800.0f, 600.0f, 1.0f, 5000.0f );

	GEnableDepthBuffer();

	GL_CHECK( glClearColor( 0.0f, 0.0f, 0.0f, 1.0f ) );
	GL_CHECK( glPointSize( 10 ) );

	camPtr->moveStep = 0.1f;

	std::stringstream out;

	out << "Num Images: %i\n"
		<< "Max Image Dims: %i\n"
		<< "Total Res: %i x %i\n"
		<< "POT Total Res: %i x %i\n";

	out << "Widths: {\n";
	for ( uint16_t w: widths.store )
	{
		out << "\t" << std::to_string( w ) << "\n";
	}

	out << "}\nHeights: {\n";
	for ( uint16_t h: heights.store )
	{
		out << '\t' << std::to_string( h ) << '\n';
	}

	out << "}\n";

	MLOG_INFO( out.str().c_str(),
			   nImage, maxImage,
			   totalRes.x, totalRes.y,
			   NextPower2( totalRes.x ), NextPower2( totalRes.y ) );
}

void TAtlas::Run( void )
{
	const float Z_PLANE = -5.0f;

	camPtr->Update();

	std::for_each( boundsList.begin(), boundsList.end(), [ & ]( const bounds_t& box )
	{
		glm::mat4 v( camPtr->ViewData().transform );

		glm::vec3 s( box.origin, Z_PLANE );
		glm::vec3 e0( ( float ) box.dimX * 0.5f, 0.0f, 0.0f );
		glm::vec3 e1( 0.0f, ( float ) box.dimY * 0.5f, 0.0f );

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

