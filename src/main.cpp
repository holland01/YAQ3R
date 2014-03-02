#include "common.h"
#include "renderer.h"
#include "log.h"
#include "tests/trenderer.h"
#include "tests/tbezsurf.h"
#include "tests/tlighting.h"

// Is global
void FlagExit( void )
{
    delete gAppTest;
    gAppTest = NULL;
}

int main( int argc, char** argv )
{
    gAppTest = new TLighting();
    gAppTest->Load();

    int code = gAppTest->Exec();

    FlagExit();

    return code;
}


