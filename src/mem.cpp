#include "common.h"
#include "log.h"

void* AllocMem( unsigned long size )
{
    void* ptr = malloc( size );

    if ( !ptr )
        ERROR( "Pointer allocated of size %iu bytes is out of memory!", size );

#ifndef NDEBUG
    memset( ptr, 0, size );
#endif

    return ptr;
}

void FreeMem( void* ptr )
{
#ifndef NDEBUG
    if ( !ptr )
        ERROR( "NULL pointer received!" );
#endif

    free( ptr );
}
