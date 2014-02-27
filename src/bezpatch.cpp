#include "bezpatch.h"

void BezPatch::Alloc( void )
{
    glGenBuffers( BEZ_BUF_COUNT, buffers );
}

void BezPatch::Tesselate( void )
{
    const int L1 = tessLevel + 1;

    for ( int i = 0; i < tessLevel; ++i )
    {
        float t = ( float )i / tessLevel;
        float s = 1 - t;


    }
}

void BezPatch::Render( void ) const
{

}
