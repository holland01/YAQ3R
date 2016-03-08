#include "bsp_data.h"
#include "glutil.h"

bool EquivalentProgramTypes( const shaderStage_t* a, const shaderStage_t* b )
{
	if ( a == b )
	{
		return true;
	}

	if ( !a ) return false;
	if ( !b ) return false;
	if ( a->alphaFunc != b->alphaFunc ) return false;
	if ( a->alphaGen != b->alphaGen ) return false;
	if ( a->blendDest != b->blendDest ) return false;
	if ( a->blendSrc != b->blendSrc ) return false;
	if ( a->depthFunc != b->depthFunc ) return false;
	//if ( a.depthPass != b.depthPass ) return false;
	if ( a->effects.size() != b->effects.size() ) return false;
	if ( a->rgbGen != b->rgbGen ) return false;

	for ( uint32_t i = 0; i < a->effects.size(); ++i )
	{
		if ( a->effects[ 0 ].name != b->effects[ 0 ].name )
		{
			return false;
		}
	}

	return true;
}

