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

	// Useful for sending legit string data (as opposed to a struct or something):
	// the emscripten function which calls into the worker requires non-const
	// char* data, which std::string can't provide, so this wrapper duplicates
	// the memory before sending it off
	void Await( em_worker_callback_func callback, const char* func,
		const std::string& strData, void* param ) const;
};

extern worker_t gFileWebWorker;

// void* destVector is expected to be a std::vector< unsgined char >*
void EM_FWW_Copy( char* data, int size, void* destVector );
void EM_FWW_Dummy( char* data, int size, void* destVector );
#endif

#endif // EMSCRIPTEN
