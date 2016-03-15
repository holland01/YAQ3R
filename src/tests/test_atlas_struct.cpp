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
	uint8_t count;
	uint16_t val;
	std::unique_ptr< bucket_t > next;
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
	newb->next = std::move( p->next );
	p->next = std::move( newb );
	p->val = v;
}

void BucketInsert( tree_t& t, uint16_t v )
{
	if ( !t.first )
	{
		t.first.reset( new bucket_t() );
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

	std::unique_ptr< bucket_t > newb( new bucket_t() );
	newb->count = 1;

	if ( prev )
	{
		if ( p )
		{
			ShiftForward( newb, p, v );
		}
		else
		{
			newb->val = v;
			prev->next = std::move( newb );
		}
	}
	else // v > first element, so we just correct that.
	{
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

void TreePoint( tree_t& t, bounds_t& bounds, float& x, float& y )
{
	if ( bounds.dimX < t.key )
	{
		x -= t.left->key;
		TreePoint( *( t.left ), bounds, x, y );
	}
	else if ( bounds.dimX > t.key )
	{
		x += t.key;
		TreePoint( *( t.right ), bounds, x, y );
	}
	else
	{
		bucket_t* prev = nullptr;
		bucket_t* curr = t.first.get();

		while ( curr && bounds.dimY < curr->val )
		{
			prev = curr;
			curr = curr->next.get();
			y += ( float )curr->val * ( float )curr->count;
		}

		// Bucket values are ordered descending, so curr->val == bounds.dimY
		y += ( float ) curr->val * ( float )( curr->count - 1 );

		curr->count--;

		x += bounds.dimX * 0.5f;
		y += bounds.dimY * 0.5f;

		if ( curr->count == 0 )
		{
			if ( prev )
			{
				prev->next = std::move( curr->next );
			}
			else
			{
				t.first = std::move( curr->next );
			}
		}
	}
}

// sort all elements by width, using buckets of heights sorted in descending order for each width node.
// use the tree as a mechanism to query

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

	median_t widths;

	for ( uint16_t i = 0; i < nImage; ++i )
	{
		bounds_t box;

		box.color = UniqueColor();
		box.dimX = NextPower2( dimGen() );
		box.dimY = NextPower2( dimGen() );

		totalRes.x += box.dimX;
		totalRes.y += box.dimY;

		widths.Insert( box.dimX );

		boundsList.push_back( box );
	}

	tree.reset( new tree_t() );
	tree->key = widths.Get();

	for ( uint16_t i = 0; i < boundsList.size(); ++i )
	{
		TreeInsert( *tree, boundsList[ i ].dimX, boundsList[ i ].dimY );
	}

	camPtr->SetPerspective( 90.0f, 800.0f, 600.0f, 1.0f, 5000.0f );

	GEnableDepthBuffer();

	GL_CHECK( glClearColor( 0.0f, 0.0f, 0.0f, 1.0f ) );
	GL_CHECK( glPointSize( 10 ) );

	camPtr->moveStep = 0.1f;

	MLOG_INFO( "Num Images: %i\n Max Image Dims: %i\n Total Res: %i x %i\n POT Total Res: %i x %i\n",
			   nImage, maxImage,
			   totalRes.x, totalRes.y );
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

