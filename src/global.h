#pragma once

#include <stdio.h>

void FlagExit( void );

template< typename T > void Swap( T& a, T& b )
{
    T tmp = a;
    a = b;
    b = tmp;
}

extern FILE* globalDrawLog;
extern FILE* globalBspDataLog;

