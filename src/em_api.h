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

	worker_t( const char* name );

	~worker_t( void );

	void Await( em_worker_callback_func callback, const char* func, char* data, int size,
		void* param ) const;
};

extern worker_t gFileWebWorker;

// void* destVector is expected to be a std::vector< unsgined char >*
void EM_FWW_Copy( char* data, int size, void* destVector );
#endif

#endif // EMSCRIPTEN
