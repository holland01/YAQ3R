#include "bsp_data.h"
#include "glutil.h"
#include "lib/cstring_util.h"

void BspData_FixupAssetPath( char* assetPath )
{
	size_t loc = StrFindLastOf( assetPath, "/" );

	if ( loc == STRING_NPOS )
	{
		return;
	}

	StrLowerN( assetPath, loc );
}

bspAssetBase_t BspData_GetAssetBaseFromPath( const char* assetStringPath, bspAssetBase_t* group )
{
	bspAssetBase_t g = BSP_ASSET_BASE_NONE;

	if ( !assetStringPath )
	{
		goto end;
	}

	{
		// Grab the path segment in between
		// the first two slashes: this is the group.
		const char* slash0 = strstr( assetStringPath, "/" );

		size_t len = slash0 - assetStringPath;

		if ( strncmp( assetStringPath, "env", len ) == 0 )
		{
			g = BSP_ASSET_BASE_ENV;
		}
		else if ( strncmp( assetStringPath, "gfx", len ) == 0 )
		{
			g = BSP_ASSET_BASE_GFX;
		}
		else if ( strncmp( assetStringPath, "models", len ) == 0 )
		{
			g = BSP_ASSET_BASE_MODELS;
		}
		else if ( strncmp( assetStringPath, "sprites", len ) == 0 )
		{
			g = BSP_ASSET_BASE_SPRITES;
		}
		else if ( strncmp( assetStringPath, "textures", len ) == 0 )
		{
			g = BSP_ASSET_BASE_TEXTURES;
		}
	}

end:
	//MLOG_INFO_ONCE( "%s -> %u", assetStringPath ? assetStringPath : "[nullptr]", g );

	if ( group )
	{
		*group = g;
	}

	return g;
}

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

//	MLOG_INFO( "%s", ss.str().c_str() );
}
