#pragma once

#include "test.h"

#ifdef EMSCRIPTEN

class IOTestWebWorker : public IOTest
{
public:
	IOTestWebWorker( void );
	~IOTestWebWorker( void );

	int operator()( void );
};

#endif
