#include "program.h"
#include "glutil.h"
#include <memory>

namespace {
	std::vector< std::unique_ptr< Program > > gProgramStorage;

	gProgramHandle_t AddProgram( Program* p )
	{
		gProgramHandle_t h =
		{
			( uint32_t ) gProgramStorage.size()
		};

		gProgramStorage.push_back( std::unique_ptr< Program >( p ) );

		return h;
	}
}

gProgramHandle_t GStoreProgram( Program* p )
{
	// Make sure we don't already have a program like this before
	// adding it
	for ( uint32_t i = 0; i < gProgramStorage.size(); ++i )
	{
		const std::unique_ptr< Program >& q = gProgramStorage[ i ];

		// Check for exact duplicate...
		if ( q.get() == p )
			return { i };

		if ( q->uniforms.size() != p->uniforms.size() )
			continue;

		if ( q->attribs.size() != p->attribs.size() )
			continue;

		auto pUnifIterator = p->uniforms.begin();
//		auto qUnifIterator = q->uniforms.begin();

		for ( ; pUnifIterator != p->uniforms.end(); ++pUnifIterator )
		{

		}
	}

	return AddProgram( p );
}


