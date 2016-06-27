#include "test_img_after_load.h"
#include "renderer/texture.h"
#include "renderer/program.h"
#include "io.h"
#include "glutil.h"

struct gDrawTest_t
{
	gTextureHandle_t mainHandle;
	gTextureHandle_t shaderHandle;
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
			
			mainHandle = GMakeTexture( makeParams );

			MLOG_ASSERT( !G_HNULL( mainHandle ), 
					"mainHandle is NULL" ); 

			std::string vertex = R"(
				in vec3 position;
				in vec2 uv;

				uniform mat4 modelToView;
				uniform mat4 viewToClip;

				varying vec2 frag_UV;

				void main(void)
				{
					gl_Position = viewToClip * modelToView * vec4( position, 
						1.0 );

					frag_UV = uv;
				}
			)";

			std::string fragment = R"(
				precision mediump float;

				varying vec2 frag_UV;
				uniform sampler2D unitSampler;

				void main(void)
				{
					gl_FragColor = texture2D( unitSampler, frag_UV ); 
				}
			)";

			std::vector< std::string > uniforms = 
			{
				"modelToView", "viewToClip", "unitSampler"
			};

			std::vector< std::string > attribs = 
			{
				"position", "uv"
			};

			Program* p = new Program( vertex, fragment, uniforms, attribs );
		}
	}
};

static std::unique_ptr< gDrawTest_t > gDrawTest( nullptr );

TTestImgAfterLoad::TTestImgAfterLoad( void )
	: Test( 800, 600, false, ASSET_Q3_ROOT"/maps/q3dm2.bsp" )
{
}

void TTestImgAfterLoad::Load( void )
{
	if ( !Test::Load( "TestImgAfterLoad" ) )
	{
		return;
	}
}

void TTestImgAfterLoad::Run( void )
{
}