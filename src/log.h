#pragma once

#include "common.h"

#pragma once

#define ERROR_INFO_STR "Call made from file %s, in function %s, on line %iu"

#define ERROR( args... )                                \
    do                                                      \
    {                                                       \
        puts("======== ERROR ========");                    \
        myPrintf( ( __func__ ), args );                   \
        puts("=======================");                    \
    }                                                       \
    while( 0 )

#define WARNING( args... )                              \
    do                                                      \
    {                                                       \
        puts("======== WARNING ========");                  \
        myPrintf( ( __func__ ), args );                   \
        puts("=======================");                    \
    }                                                       \
    while( 0 )

#define ASSERT( condition, args... )    \
    do                                      \
    {                                       \
        if ( !( condition ) )               \
        {                                   \
            ERROR( args );                  \
        }                                   \
    }                                       \
    while( 0 )

void myPrintf( const char* header, const char* fmt, ... );

void myDateTime( const char* format, char* outBuffer, int length );
