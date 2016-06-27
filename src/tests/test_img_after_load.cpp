#include "test_img_after_load.h"
#include "renderer/texture.h"
#include "renderer/program.h"

struct gDrawTest_t
{
	gTextureHandle_t mainHandle, shaderHandle;
	gSamplerHandle_t sampler;

	gProgramHandle_t program;

	gDrawTest_t( gImageParamList_t& mainImages,
		gImageParamList_t& shaderImages,
		gSamplerHandle_t sampler )
	{
		UNUSED( shaderImages );

		{
			gTextureMakeParams_t makeParams( mainImages,
					sampler, 0 );	
			
			mainHandle = GMakeTexture(makeParams);

			MLOG_ASSERT( !G_HNULL(mainHandle), 
					"mainHandle is NULL" ); 
		}
		
	}
};