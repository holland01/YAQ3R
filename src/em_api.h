#pragma once

#ifdef EMSCRIPTEN

#include "common.h"
#include <emscripten.h>

void EM_UnmountFS( void );

void EM_MountFS( void );

#ifdef EM_USE_WORKER_THREAD

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

	void Await( em_worker_callback_func callback, const char* func, char* data, int size,
		void* param ) const
	{
	//	MLOG_ASSERT( callback, "null callback isn't allowed; it's required for "\
	//		"determining the size of the work queue - pass a dummy if necessary." );

		int prevQueueSize = emscripten_get_worker_queue_size( handle );
		emscripten_call_worker( handle, func, data, size, callback, param );
		
/*
		volatile int dontOptimizeMe = 0;
		while ( emscripten_get_worker_queue_size( handle ) > prevQueueSize )
		{
			dontOptimizeMe++;
		}
		*/
	}
};

extern worker_t gFileWebWorker;

// void* destVector is expected to be a std::vector< unsgined char >*
void EM_FWW_Copy( char* data, int size, void* destVector );
#endif

#endif // EMSCRIPTEN
