#include "q3m_model.h"
#include "glutil.h"

bezPatch_t::bezPatch_t( void )
	: vbo( 0 ),
	  subdivLevel( 0 )
{
	memset( controlPoints, 0, sizeof( controlPoints ) );
}

bezPatch_t::~bezPatch_t( void )
{
	GL_CHECK( glDeleteBuffers( 1, &vbo ) );
}

mapModel_t::mapModel_t( void )
	: vao( 0 )
{
}

mapModel_t::~mapModel_t( void )
{
	if ( vao )
	{
		GL_CHECK( glDeleteVertexArrays( 1, &vao ) );
	}

	for ( bezPatch_t* patch: patches )
	{
		delete patch;
	}
}

