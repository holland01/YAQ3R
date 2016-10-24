#include "program.h"
#include "bsp_data.h"
#include "glutil.h"
#include <memory>

// Storage for dynamic programs

namespace {
	std::vector< std::unique_ptr< Program > > gProgramStorage;

	INLINE gProgramHandle_t AddProgram( Program* p )
	{
		gProgramHandle_t h = GFindProgramByData( p->attribs,
			p->uniforms, p->stage );

		if ( !G_HNULL( h ) )
		{
			return h;
		}
			
		h.id = ( uint32_t ) gProgramStorage.size();

		gProgramStorage.push_back( std::unique_ptr< Program >( p ) );

		return h;
	}

	bool CompareMaps( const programDataMap_t& p, const programDataMap_t& q )
	{
		if ( p.size() != q.size() )
			return false;

		for ( const auto& pIterator: p )
		{
			bool found = false;
			for ( const auto& qIterator: q )
			{
				if ( pIterator.first == qIterator.first )
				{
					found = true;
					break;
				}
			}

			// If this isn't true, we're definitely wasting our time
			// Since we can only check pIterator->first once
			if ( !found )
			{
				return false;
			}
		}

		return true;
	}
}

gProgramHandle_t GFindProgramByData( const programDataMap_t& attribs,
									 const programDataMap_t& uniforms,
									 const shaderStage_t* stage )
{
	for ( uint32_t i = 0; i < gProgramStorage.size(); ++i )
	{
		const std::unique_ptr< Program >& q = gProgramStorage[ i ];

		if ( CompareMaps( q->attribs, attribs )
		  && CompareMaps( q->uniforms, uniforms )
		  && EquivalentProgramTypes( stage, q->stage ) )
		{
			return { i };
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

	return AddProgram( p );
}

Program* GQueryProgram( gProgramHandle_t handle )
{
	if ( handle.id >= gProgramStorage.size() )
	{
		return NULL;
	}

	return gProgramStorage[ handle.id ].get();
}

size_t GNumPrograms( void )
{
	return gProgramStorage.size();
}
