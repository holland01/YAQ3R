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
	if ( a->depthPass != b->depthPass ) return false;
	if ( a->effects.size() != b->effects.size() ) return false;
	if ( a->rgbGen != b->rgbGen ) return false;

	for ( uint32_t i = 0; i < a->effects.size(); ++i )
	{
		bool found = false;
		for ( uint32_t j = 0; j < b->effects.size() && !found; ++j )
		{
			if ( a->effects[ i ].name == b->effects[ j ].name )
			{
				found = true;
			}
		}

		if (!found)
		{
			return false;
		}
	}

	return true;
}

void shaderInfo_t::PrintStageTextureNames( void ) const
{
	std::stringstream ss;

	ss << "Stage Buffer Size: " << stageBuffer.size() << "\n";

	ss << &name[0] << "\n";

	uint32_t i = 0;
	for ( const shaderStage_t& stage: stageBuffer )
	{
		ss << "[" << i++ << "]: ";
		if ( stage.mapType == MAP_TYPE_IMAGE )
		{
			ss << &stage.texturePath[0];
		}
		else
		{
			switch ( stage.mapType )
			{
				case MAP_TYPE_LIGHT_MAP:
					ss << "$lightmap";
					break;
				case MAP_TYPE_WHITE_IMAGE:
					ss << "$whiteimage";
					break;
				default:
					ss << "$unknown";
					break;
			}
		}
		ss << "\n";
	}

	MLOG_INFO( "%s", ss.str().c_str() );
}
