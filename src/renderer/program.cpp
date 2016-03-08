#include "program.h"
#include "glutil.h"
#include <memory>

// Storage for dynamic programs

namespace {
	std::vector< std::unique_ptr< Program > > gProgramStorage;

	INLINE gProgramHandle_t AddProgram( Program* p )
	{
		gProgramHandle_t h =
		{
			( uint32_t ) gProgramStorage.size()
		};

		gProgramStorage.push_back( std::unique_ptr< Program >( p ) );

		return h;
	}

	bool CompareMaps( const Program::dataMap_t& p, const Program::dataMap_t& q )
	{
		if ( p.size() != q.size() )
			return false;

		std::unordered_map< std::string, uint8_t > vals;

		for ( const auto& pIterator: p )
		{
			bool found = false;
			for ( const auto& qIterator: q )
			{
				if ( pIterator.first == qIterator.first )
				{
					// Only early out if we haven't actually come across this (successful) comparison
					// yet, otherwise we duplicate successes and eliminate the potential for a valid find
					if ( vals.find( pIterator.first ) == vals.end() )
					{
						vals[ pIterator.first ] = 1;
						found = true;
						break;
					}
				}
			}

			// If this isn't true, we're definitely wasting our time
			// Since we can only check pIterator->first once
			if ( !found )
			{
				return false;
			}
		}

		return vals.size() == p.size();
	}
}

gProgramHandle_t GFindProgramByData( const Program::dataMap_t& attribs, const Program::dataMap_t& uniforms )
{
	for ( uint32_t i = 0; i < gProgramStorage.size(); ++i )
	{
		const std::unique_ptr< Program >& q = gProgramStorage[ i ];

		if ( CompareMaps( q->attribs, attribs ) )
		{
			if ( CompareMaps( q->uniforms, uniforms ) )
			{
				return { i };
			}
		}
	}

	return { G_UNSPECIFIED };
}

gProgramHandle_t GStoreProgram( Program* p )
{
	if ( !p )
	{
		return { G_UNSPECIFIED };
	}

	// Make sure we don't already have a program like this before
	// adding it

	return AddProgram( p );
}

Program* GQueryProgram( const gProgramHandle_t& handle )
{
	if ( handle.id >= gProgramStorage.size() )
	{
		return NULL;
	}

	return gProgramStorage[ handle.id ].get();
}
