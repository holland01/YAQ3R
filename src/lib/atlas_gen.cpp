
#include "atlas_gen.h"
#include "stats.h"
#include "renderer/texture.h"
#include "math.h"
#include <memory>
#include <utility>

struct bounds_t
{
	uint16_t dimX, dimY;
	glm::vec2 origin;
};

struct atlasBucket_t
{
	uint16_t count = 0;
	uint16_t val = 0;
	std::unique_ptr< atlasBucket_t > next;

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
		return ( uint8_t )( count & 0xFF );
	}

	void SubOffset( void )
	{
		uint16_t off = ( count >> 8 ) & 0xFF;
		off--;
		count = ( count & 0xFF ) | ( off << 8 );
	}

	uint8_t ReadOffset( void )
	{
		return ( uint8_t ) ( ( count >> 8 ) & 0xFF );
	}
};

struct atlasTree_t
{
	uint16_t key;
	std::unique_ptr< atlasBucket_t > first;
	std::unique_ptr< atlasTree_t > left;
	std::unique_ptr< atlasTree_t > right;
};

namespace {

void ShiftForward( std::unique_ptr< atlasBucket_t >& newb, atlasBucket_t* p, uint16_t v )
{
	newb->val = p->val;
	newb->count = p->count;
	newb->next = std::move( p->next );
	p->next = std::move( newb );
	p->val = v;
	p->WriteCount( 1 );
}

// Insert height values in descending order
void BucketInsert( atlasTree_t& t, uint16_t v )
{
	if ( !t.first )
	{
		t.first.reset( new atlasBucket_t() );
		t.first->val = v;
		t.first->count = 1;
		return;
	}

	atlasBucket_t* prev, *p;

	prev = nullptr;
	p = t.first.get();

	while ( p && v < p->val )
	{
		prev = p;
		p = p->next.get();
	}

	if ( p && p->val == v )
	{
		p->IncCount();
		return;
	}

	if ( prev )
	{
		std::unique_ptr< atlasBucket_t > newb( new atlasBucket_t() );

		// p->val < v < prev->val
		if ( p )
		{
			ShiftForward( newb, p, v );
		}
		else
		{
			newb->val = v;
			newb->WriteCount( 1 );
			prev->next = std::move( newb );
		}
	}
	// if prev == nullptr, p automatically isn't null
	// and v > first element, so we just correct that.
	else
	{
		std::unique_ptr< atlasBucket_t > newb( new atlasBucket_t() );
		ShiftForward( newb, p, v );
	}
}

std::unique_ptr< atlasTree_t > TreeMake( uint16_t k, uint16_t v )
{
	std::unique_ptr< atlasTree_t > t( new atlasTree_t() );
	t->key = k;
	t->first.reset( new atlasBucket_t() );
	t->first->val = v;
	t->first->WriteCount( 1 );
	return t;
}

void TreeInsert( atlasTree_t& t, uint16_t k, uint16_t v );

void InsertOrMake( std::unique_ptr< atlasTree_t >& t, uint16_t k, uint16_t v )
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

void TreeInsert( atlasTree_t& t, uint16_t k, uint16_t v )
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

// The ReadCount and ReadOffset methods will initially return the same values. Things change, however,
// when the image's height corresponds to a particular bucket: the offset portion will decrement
// for each image which has a matching height value. The initial count bits will remain the same, to ensure
// that images of different heights will not "invade" the space of the given image these parameters correspond
// to.
void TreePoint( atlasTree_t& t, atlasPositionMap_t& map )
{
	if ( map.image->width < t.key )
	{
		TreePoint( *( t.left ), map );
	}
	else if ( map.image->width > t.key )
	{
		TreePoint( *( t.right ), map );
	}
	else
	{
		atlasBucket_t* curr = t.first.get();

		// offset our origin through the summation of all heights which
		// are placed before the height section the owning image belongs to;
		// take into account the amount of duplications for every height value
		while ( curr && map.image->height < curr->val )
		{
			map.origin.y += ( float )curr->val * ( float )curr->ReadCount();
			curr = curr->next.get();
		}

		// if curr != nullptr, we know that, by nature of the this structure,
		// curr->val is implicitly == image.width
		if ( curr && curr->ReadOffset() > 1 )
		{
			curr->SubOffset();
			map.origin.y += ( float )curr->val * ( float )( curr->ReadOffset() );
		}

		//map.origin.x += map.image->width * 0.5f;
		//map.origin.y += map.image->height * 0.5f;
	}
}

// For lists of images which have varying sizes
std::vector< atlasPositionMap_t > AtlasGenVariedOrigins( const std::vector< gImageParams_t >& params )
{
	atlasTree_t rootTree;
	median_t< uint16_t > widths;

	for ( uint16_t i = 0; i < params.size(); ++i )
	{
		widths.Insert( params[ i ].width );
	}

	rootTree.key = widths.GetMedian();

	for ( const gImageParams_t& param: params )
	{
		TreeInsert( rootTree, param.width, param.height );
	}

	std::vector< atlasPositionMap_t > posMap;

	for ( const gImageParams_t& image: params )
	{
		atlasPositionMap_t pmap;
		pmap.origin.x = image.width;
		pmap.image = &image;
		TreePoint( rootTree, pmap );
		posMap.push_back( pmap );
	}

	return std::move( posMap );
}

// For lists of images which all have the same dimensions
std::vector< atlasPositionMap_t > AtlasGenUniformOrigins( const std::vector< gImageParams_t >& params )
{
	uint16_t square = NextSquare( params.size() );

	std::vector< atlasPositionMap_t > posMap;

	for ( uint16_t y = 0; y < square; ++y )
	{
		for ( uint16_t x = 0; x < square; ++x )
		{
			uint16_t slot = y * square + x;

			if ( slot >= params.size() )
			{
				break;
			}

			atlasPositionMap_t map;
			map.image = &params[ slot ];
			map.origin = glm::vec2( x * params[ slot ].width, y * params[ slot ].height );
			posMap.push_back( map );
		}
	}

	return std::move( posMap );
}

} // end namespace

std::vector< atlasPositionMap_t > AtlasGenOrigins( const std::vector< gImageParams_t >& params )
{
	// Determine our atlas layout
	for ( uint16_t i = 1; i < params.size(); ++i )
	{
		// If true, we know that there is at least one image with differing
		// dimensions from the rest, so we take that into account...
		if ( params[ i - 1 ].width != params[ i ].width
			 || params[ i - 1 ].height != params[ i ].height )
		{
			return std::move( AtlasGenVariedOrigins( params ) );
		}
	}

	return std::move( AtlasGenUniformOrigins( params ) );
}
