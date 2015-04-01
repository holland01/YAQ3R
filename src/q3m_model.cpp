#include "q3m_model.h"
#include "glutil.h"

deformModel_t::deformModel_t( void )
	: vbo( 0 ),
	  ibo( 0 )
{
}

deformModel_t::~deformModel_t( void )
{
	if ( vbo )
	{
		DelBufferObject( GL_ARRAY_BUFFER, &vbo, 1 );
	}

	if ( ibo )
	{
		DelBufferObject( GL_ELEMENT_ARRAY_BUFFER, &ibo, 1 );
	}
}
