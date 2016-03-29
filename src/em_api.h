#pragma once

#ifdef EMSCRIPTEN

#include "common.h"
#include <emscripten.h>

struct worker_t
{
	worker_handle handle;

	worker_t( const char* name )
		: handle( emscripten_create_worker( name ) )
	{
	}

	~worker_t( void )
	{
		if ( handle )
		{
			emscripten_destroy_worker( handle );
		}
	}
};

void EM_UnmountFS( void );

void EM_MountFS( void );

extern worker_t gFileWebWorker;

#endif // EMSCRIPTEN
