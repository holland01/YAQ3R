#include "q3m_model.h"
#include "glutil.h"

mapModel_t::mapModel_t( void )
	: deform( false ),
	  vbo( 0 ),
	  subdivLevel( 0 )
{
}

mapModel_t::~mapModel_t( void )
{
	if ( vbo )
	{
		DeleteBufferObject( GL_ARRAY_BUFFER, vbo );
	}
}

