
#include "atlas_gen.h"
#include "io.h"
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

	uint16_t TotalBuckets( void )
	{
		uint16_t thisCount = ReadCount();

		if ( next )
		{
			thisCount += next->TotalBuckets();
		}

		return thisCount;
	}

	std::string Info( void )
	{
		std::stringstream ss;
		uint32_t c = ReadCount();
		ss << "Height: " << val <<  "\nCount: " << c << "\n";

		if ( next )
		{
			ss << next->Info();
		}

		std::string result ( ss.str() );

		return result;
	}

	atlasBucket_t* FindRange( uint16_t offset, uint16_t i = 0/*, atlasBucket_t** prev*/ )
	{
		uint32_t upperBound = i + ReadCount();

		if ( i <= offset )
		{
			if ( next )
			{
				if ( offset < upperBound )
				{
					return this;
				}

				//*prev = this;

				return next->FindRange( offset, upperBound/*, prev*/ );
			}

			return this;
		}
		else if ( next )
		{
			//*prev = this;

			return next->FindRange( offset, upperBound/*, prev*/ );
		}

		return nullptr;
	}
};

struct atlasTree_t
{
	uint16_t key;
	std::unique_ptr< atlasTree_t > left;
	std::unique_ptr< atlasTree_t > right;

	std::vector< std::unique_ptr< atlasBucket_t > > columns;

	std::unique_ptr< atlasBucket_t >& First( void )
	{
		return columns[ 0 ];
	}
};

struct slotMetrics_t
{
	uint16_t width;
	uint16_t numBuckets;
};

struct atlasTreeMetrics_t
{
	slotMetrics_t highest;		// the width "slot" which holds the maximum height value
	slotMetrics_t nextHighest;	// same thing, but the second largest
	uint16_t base;				// each width category, summed
};

namespace {

struct meta_t
{
	LogHandle log;

	meta_t( void )
		: log( "log/atlas_gen.txt", true )
	{
	}

	void LogData( const atlasTreeMetrics_t& metrics, atlasTree_t& treeRoot )
	{
		O_LogF( log.ptr, "METRICS", "\n\thighest: %i\n\tnextHighest: %i\n\tbase: %i\n\n\n\n",
				metrics.highest, metrics.nextHighest, metrics.base );

		LogData_r( &treeRoot );
	}

	void LogData_r( atlasTree_t* t )
	{
		if ( t )
		{
			LogData_r( t->left.get() );

			std::string info( t->First()->Info() );
			O_LogF( log.ptr, "entry", "\nWidth: %i\n Bucket (Column) Count: %i\n Bucket Info: \n\n%s\n",
					t->key, t->columns.size(), info.c_str() );

			LogData_r( t->right.get() );
		}
	}
};

std::unique_ptr< meta_t > gMeta( new meta_t() );

void ShiftForward( std::unique_ptr< atlasBucket_t >& newb, atlasBucket_t* p, uint16_t v )
{
	newb->val = p->val;
	newb->count = p->count;
	newb->next = std::move( p->next );
	p->next = std::move( newb );
	p->val = v;
	p->WriteCount( 1 );
}

// Insert height values in descending order ( largest value is at the bottom )
void BucketInsert( atlasTree_t& t, uint16_t v )
{
	if ( t.columns.empty() )
	{
		std::unique_ptr< atlasBucket_t > f( new atlasBucket_t() );
		f->val = v;
		f->count = 1;
		t.columns.push_back( std::move( f ) );
		return;
	}

	atlasBucket_t* prev, *p;

	prev = nullptr;
	p = t.First().get();

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

	std::unique_ptr< atlasBucket_t > buck( new atlasBucket_t() );

	buck->val = v;
	buck->WriteCount( 1 );

	t->columns.push_back( std::move( buck ) );

	return t;
}

void TreeInsert( atlasTree_t& t, uint16_t k, uint16_t v, atlasTreeMetrics_t& metrics );

void InsertOrMake( std::unique_ptr< atlasTree_t >& t, uint16_t k, uint16_t v, atlasTreeMetrics_t& metrics )
{
	if ( t )
	{
		TreeInsert( *t, k, v, metrics );
	}
	else
	{
		t = TreeMake( k, v );
	}
}

void TreeInsert( atlasTree_t& t, uint16_t k, uint16_t v, atlasTreeMetrics_t& metrics )
{
	if ( k < t.key )
	{
		InsertOrMake( t.left, k, v, metrics );
	}
	else if ( k > t.key )
	{
		InsertOrMake( t.right, k, v, metrics );
	}
	else
	{
		BucketInsert( t, v );

		uint16_t totalBuckets = t.First()->TotalBuckets();

		if ( totalBuckets > metrics.highest.numBuckets )
		{
			metrics.highest.width = k;
			metrics.highest.numBuckets = totalBuckets;
		}
		else if ( totalBuckets > metrics.nextHighest.numBuckets )
		{
			metrics.nextHighest.width = k;
			metrics.nextHighest.numBuckets = totalBuckets;
		}
	}
}

bool TraverseColumn( atlasTree_t& t, atlasPositionMap_t& map, uint8_t index )
{
	map.origin.y = 0;

	atlasBucket_t* curr = t.columns[ index ].get();

	// offset our origin through the summation of all heights which
	// are placed before the height section the owning image belongs to;
	// take into account the amount of duplications for every height value
	while ( curr && map.image->height < curr->val )
	{
		map.origin.y += ( float )curr->val * ( float )curr->ReadCount();
		curr = curr->next.get();
	}

	// if curr != nullptr, we know that, by nature of the this structure,
	// curr->val is implicitly == image.height
	if ( curr && curr->ReadOffset() > 1 )
	{
		curr->SubOffset();
		map.origin.y += ( float )curr->val * ( float )curr->ReadOffset();
	}

	return !!curr;
}

uint16_t SumBounds( const atlasTree_t* t, uint16_t target )
{
	uint16_t s = 0;

	if ( t )
	{
		s += SumBounds( t->left.get(), target );

		if ( t->key < target )
		{
			s += t->key * t->columns.size();
		}

		s += SumBounds( t->right.get(), target );
	}

	return s;
}

// The ReadCount and ReadOffset methods will initially return the same values. Things change, however,
// when the image's height corresponds to a particular bucket: the offset portion will decrement
// for each image which has a matching height value. The initial count bits will remain the same, to ensure
// that images of different heights will not "invade" the space of the given image these parameters correspond
// to.
void TreePoint( atlasTree_t* t, atlasPositionMap_t& map, const atlasTree_t* root )
{
	if ( t )
	{
		if ( map.image->width < t->key )
		{
			TreePoint( t->left.get(), map, root );
		}
		else if ( map.image->width > t->key )
		{
			TreePoint( t->right.get(), map, root );
		}
		else
		{
			volatile bool found = false;

			uint32_t i;

			for ( i = 0; i < t->columns.size() && !found; )
			{
				found = TraverseColumn( *t, map, i );

				if ( !found )
				{
					++i;
				}
			}

			assert( found );

			map.origin.x = SumBounds( root, t->key ) + i * t->key;
		}
	}
}

atlasTree_t* TreeFetch( atlasTree_t* t, uint16_t key )
{
	if ( t )
	{
		if ( key < t->key )
		{
			return TreeFetch( t->left.get(), key );
		}
		else if ( key > t->key )
		{
			return TreeFetch( t->right.get(), key );
		}
	}

	return t;
}

// For lists of images which have varying sizes
std::vector< atlasPositionMap_t > AtlasGenVariedOrigins(
		const std::vector< gImageParams_t >& params )
{
	atlasTree_t rootTree;
	median_t< uint16_t > widths;

	for ( uint16_t i = 0; i < params.size(); ++i )
	{
		widths.Insert( params[ i ].width );
	}

	rootTree.key = widths.GetMedian();

	atlasTreeMetrics_t metrics =
	{
		{ 0, 0 },
		{ 0, 0 },
		0
	};

	metrics.base = widths.Sum();

	for ( const gImageParams_t& param: params )
	{
		TreeInsert( rootTree, param.width, param.height, metrics );
	}

	// Check to see if our highest bucket count is two standard deviations
	// from the nextHighest. If so, we should split any bucket groups which
	// are significantly high into separate columns: this will significantly
	// alleviate any potential problems with attempting a texture allocation
	// which is larger than GL_MAX_TEXTURE_SIZE
	if ( metrics.highest.numBuckets >= ( metrics.nextHighest.numBuckets << 1 ) )
	{
		atlasTree_t* t = TreeFetch( &rootTree, metrics.highest.width );

		uint16_t cutoff = metrics.highest.numBuckets >> 1;

		atlasBucket_t* current = t->First()->FindRange( cutoff - 1 );

		t->columns.push_back( std::move( current->next ) );

		current->next.reset( nullptr );
	}

	std::vector< atlasPositionMap_t > posMap;

	for ( const gImageParams_t& image: params )
	{
		atlasPositionMap_t pmap;
		pmap.image = &image;
		TreePoint( &rootTree, pmap, &rootTree );
		posMap.push_back( pmap );
	}

	gMeta->LogData( metrics, rootTree );

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
