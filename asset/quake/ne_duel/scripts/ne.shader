textures/ne/ik_sky_day
{
	surfaceparm noimpact
	surfaceparm nolightmap
	surfaceparm sky
	qer_editorimage textures/ikbase_sky/ik_sky_day_back.tga
	q3map_lightimage textures/ikbase_sky/ik_sky_day_back.tga
	q3map_surfacelight 740
	q3map_sun	1 0.9 0.8 80	240 60
	skyparms - 512 -
	{
		map textures/ikbase_sky/ik_sky_day_back.tga
		tcMod scale 2 2
		tcMod scroll 0.01 0.01
		depthWrite
	}
	{
		map textures/ikbase_sky/ik_sky_day_front.tga
		blendFunc GL_ONE GL_ONE
		tcMod scale 3 3
		tcMod scroll 0.02 0.02
	}
}

textures/ne/e8_jumppad_1
{
	surfaceparm nodamage
	q3map_lightimage textures/sfx/jumppadsmall.tga	
	q3map_surfacelight 400

	
	{
		map textures/ne/e8_jumppad_1.jpg
		rgbGen identity
	}
	
	{
		map $lightmap
		rgbGen identity
		blendfunc gl_dst_color gl_zero
	}
	
	{
		map textures/sfx/bouncepad01b_layer1.tga
		blendfunc gl_one gl_one
		rgbGen wave sin .5 .5 0 1.5	
	}

	{
		clampmap textures/sfx/jumppadsmall.tga
		blendfunc gl_one gl_one
		tcMod stretch sin 1.2 .8 0 1.5
		rgbGen wave square .5 .5 .25 1.5
	}

}

textures/ne/ne_base_door
{

	{
		map textures/ne/ne_base_door_chrome.tga
                tcGen environment
		tcmod scale .25 .25 
		rgbGen identity   
	}

		
	{
		map textures/ne/ne_base_door.tga
		blendFunc GL_ONE_MINUS_SRC_ALPHA GL_SRC_ALPHA
		rgbGen identity
	}

	{
		map textures/base_door/quake3sign_outside.tga
		blendFunc GL_ONE_MINUS_SRC_ALPHA GL_SRC_ALPHA
		rgbGen identity
	}


	{
		map $lightmap
		rgbgen identity
		blendFunc GL_DST_COLOR GL_ZERO
	}	
		
} 