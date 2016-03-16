
#include "atlas_gen.h"
#include <memory>

struct bounds_t
{
	uint16_t dimX, dimY;
	glm::vec2 origin;
	glm::u8vec4 color;
	bool rot = false;
};

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

struct tree_t
{
	uint16_t key;
	std::unique_ptr< bucket_t > first;
	std::unique_ptr< tree_t > left;
	std::unique_ptr< tree_t > right;
};

namespace {

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
		p->IncCount();
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
			newb->WriteCount( 1 );
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
	t->first->WriteCount( 1 );
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
			bounds.origin.y += ( float )curr->val * ( float )curr->ReadCount();
			curr = curr->next.get();
		}

		// if curr != nullptr, we know that, by nature of the this structure,
		// curr->val is implicitly == bounds.dimY
		if ( curr && curr->count > 1 )
		{
			curr->SubOffset();
			bounds.origin.y += ( float )curr->val * ( float )( curr->ReadOffset() );
		}

		bounds.origin.x += sign * bounds.dimX * 0.5f;
		bounds.origin.y += bounds.dimY * 0.5f;
	}
}

}
