#include "common.h"
#include "renderer.h"
#include "log.h"
#include "tests/trenderer.h"
#include "tests/tbezsurf.h"
#include "tests/jpeg.h"

// Is global
void FlagExit( void )
{
    delete gAppTest;
    gAppTest = NULL;
}

int main( int argc, char** argv )
{
    gAppTest = new TRenderer();
    gAppTest->Load();

    int code = gAppTest->Exec();

    FlagExit();

    return code;
}


