//**********************************************************************
//
//	Title  : egyptsoc.shader
//	Author : Sock
//	Date   : 1st July 2001
//	Email  : sock@planetquake.com
//	URL    : http://www.planetquake.com/simland
//	Version: 1.6
//
//  If you use any of these Egyptian shader/textures I kindly ask
//  YOU to give me credit for my work within your README file or
//  TEXT file distributed with your map/mod.
//
//**********************************************************************
//
// Index of shaders (128) :-
//
// (32) 32x32 light sources : 01 = Cross, 02 = Triangle, 03 = Star, 04 = Flower
//
// lig_032-01b1-2k  lig_032-02b1-2k  lig_032-03b1-2k  lig_032-04b1-2k  - Blue + grey bckgrd
// lig_032-01b2-2k  lig_032-02b2-2k  lig_032-03b2-2k  lig_032-04b2-2k  - Blue + yellow bckgrd
// lig_032-01r1-2k  lig_032-02r1-2k  lig_032-03r1-2k  lig_032-04r1-2k  - Red + grey bckgrd
// lig_032-01r2-2k  lig_032-02r2-2k  lig_032-03r2-2k  lig_032-04r2-2k  - Red + yellow bckgrd
// lig_032-01y1-2k  lig_032-02y1-2k  lig_032-03y1-2k  lig_032-04y1-2k  - Yellow + grey bckgrd
// lig_032-01y2-2k  lig_032-02y2-2k  lig_032-03y2-2k  lig_032-04y2-2k  - Yellow + yellow bckgrd
// lig_032-01w1-2k  lig_032-02w1-2k  lig_032-03w1-2k  lig_032-04w1-2k  - White + grey bckgrd
// lig_032-01w2-2k  lig_032-02w2-2k  lig_032-03w2-2k  lig_032-04w2-2k  - White + yellow bckgrd
//
// (40) 64x64 light sources : 01 = Anhk, 02 = Circle, 04 = Cross, 05 = Square, 06 = Triangle
//
// lig_064-01b1-2k  lig_064-02b1-2k  lig_064-04b1-2k  lig_064-05b1-2k  lig_064-06b1-2k  - Blue + grey bckgrd
// lig_064-01b2-2k  lig_064-02b2-2k  lig_064-04b2-2k  lig_064-05b2-2k  lig_064-06b2-2k  - Blue + yellow bckgrd
// lig_064-01r1-2k  lig_064-02r1-2k  lig_064-04r1-2k  lig_064-05r1-2k  lig_064-06r1-2k  - Red + grey bckgrd
// lig_064-01r2-2k  lig_064-02r2-2k  lig_064-04r2-2k  lig_064-05r2-2k  lig_064-06r2-2k  - Red + yellow bckgrd
// lig_064-01y1-2k  lig_064-02y1-2k  lig_064-04y1-2k  lig_064-05y1-2k  lig_064-06y1-2k  - Yellow + grey bckgrd
// lig_064-01y2-2k  lig_064-02y2-2k  lig_064-04y2-2k  lig_064-05y2-2k  lig_064-06y2-2k  - Yellow + yellow bckgrd
// lig_064-01w1-2k  lig_064-02w1-2k  lig_064-04w1-2k  lig_064-05w1-2k  lig_064-06w1-2k  - White + grey bckgrd
// lig_064-01w2-2k  lig_064-02w2-2k  lig_064-04w2-2k  lig_064-05w2-2k  lig_064-06w2-2k  - White + yellow bckgrd
//
// (18) 256x64 Band style light : 01 = Single, 02 = ZigZag, 03 = Triangle
//
// lig_b064-01a  lig_b064-02a  lig_b064-03a  - Blue + grey bckgrd
// lig_b064-01b  lig_b064-02b  lig_b064-03b  - Blue + yellow bckgrd
// lig_b064-01c  lig_b064-02c  lig_b064-03c  - Yellow + grey bckgrd
// lig_b064-01d	 lig_b064-02d  lig_b064-03d  - Yellow + yellow bckgrd
// lig_b064-01e  lig_b064-02e  lig_b064-03e  - Red + grey bckgrd
// lig_b064-01f	 lig_b064-02f  lig_b064-03f  - Red + yellow bckgrd
//
// (06) Vertical style light - 2 bar with additional border 96x192
//
// lig_v192-01ba  - Blue + grey bckgrd
// lig_v192-01bb  - Blue + yellow bckgrd
// lig_v192-01ya  - Yellow + grey bckgrd
// lig_v192-01yb  - Yellow + yellow bckgrd
// lig_v192-01wa  - White + grey bckgrd
// lig_v192-01wb  - White + yellow bckgrd
//
// (06) Weapon Markers 1 - 2 rot swirls + FAST glowing symbols
//
// wmblue_floor1a  wmblue_floor1b  - Blue symbols and swirls
// wmgold_floor1a  wmgold_floor1b  - Gold symbols and swirls
// wmred_floor1a   wmred_floor1b   - Red symbols and swirls
//
// (06) Jump Pads 1 - 1 rot swirl + 1 jumppad stretch + SLOW glowing symbols
//
// jpblue_floor1a  jpblue_floor1b  - Blue symbols and swirls
// jpgold_floor1a  jpgold_floor1b  - Gold symbols and swirls
// jpred_floor1a   jpred_floor1b   - Red symbols and swirls
//
// (12) Weapon Markers with Glowing central areas
//
// s128-01wc  s128-02wc  - Blue + grey bckgrd
// s128-01wd  s128-02wd  - Blue + yellow bckgrd
// s128-01we  s128-02we  - Gold + grey bckgrd
// s128-01wcr s128-02wcr - Red + grey bckgrd
// s128-01wdr s128-02wdr - Red + yellow bckgrd
//
// (08) Grates in grey/yellow to match other tiles
//
// grate1a  grate1b  - square 32x32 design
// grate2a  grate2b  - small rectangle design
// grate3a  grate3b  - ring 32x32 design
// grate4a  grate4b  - detail version of ring design
//

//**********************************************************************//
// LIGHT 32 - 01 : Cross design light source				//
//**********************************************************************//
textures/egyptsoc_sfx/lig_032-01b1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-01b1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-01b.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-01b1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-01b.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}


textures/egyptsoc_sfx/lig_032-01b2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-01b2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-01b.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-01b2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-01b.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_032-01r1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-01r1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-01r.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-01r1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-01r.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}


textures/egyptsoc_sfx/lig_032-01r2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-01r2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-01r.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-01r2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-01r.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_032-01y1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-01y1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-01y.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-01y1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-01y.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_032-01y2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-01y2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-01y.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-01y2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-01y.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_032-01w1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-01w1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-01w.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-01w1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-01w.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_032-01w2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-01w2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-01w.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-01w2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-01w.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

//**********************************************************************//
// LIGHT 32 - 02 : Triangle light source				//
//**********************************************************************//
textures/egyptsoc_sfx/lig_032-02b1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-02b1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-02b.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-02b1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-02b.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}


textures/egyptsoc_sfx/lig_032-02b2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-02b2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-02b.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-02b2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-02b.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_032-02r1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-02r1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-02r.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-02r1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-02r.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}


textures/egyptsoc_sfx/lig_032-02r2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-02r2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-02r.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-02r2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-02r.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_032-02y1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-02y1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-02y.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-02y1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-02y.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_032-02y2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-02y2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-02y.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-02y2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-02y.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_032-02w1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-02w1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-02w.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-02w1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-02w.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_032-02w2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-02w2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-02w.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-02w2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-02w.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

//**********************************************************************//
// LIGHT 32 - 03 : Star design light source				//
//**********************************************************************//
textures/egyptsoc_sfx/lig_032-03b1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-03b1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-03b.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-03b1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-03b.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}


textures/egyptsoc_sfx/lig_032-03b2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-03b2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-03b.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-03b2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-03b.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_032-03r1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-03r1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-03r.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-03r1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-03r.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}


textures/egyptsoc_sfx/lig_032-03r2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-03r2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-03r.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-03r2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-03r.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_032-03y1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-03y1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-03y.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-03y1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-03y.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_032-03y2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-03y2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-03y.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-03y2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-03y.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_032-03w1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-03w1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-03w.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-03w1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-03w.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_032-03w2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-03w2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-03w.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-03w2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-03w.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

//**********************************************************************//
// LIGHT 32 - 04 : Flower design light source				//
//**********************************************************************//
textures/egyptsoc_sfx/lig_032-04b1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-04b1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-04b.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-04b1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-04b.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}


textures/egyptsoc_sfx/lig_032-04b2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-04b2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-04b.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-04b2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-04b.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_032-04r1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-04r1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-04r.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-04r1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-04r.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}


textures/egyptsoc_sfx/lig_032-04r2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-04r2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-04r.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-04r2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-04r.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_032-04y1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-04y1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-04y.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-04y1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-04y.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_032-04y2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-04y2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-04y.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-04y2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-04y.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_032-04w1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-04w1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-04w.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-04w1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-04w.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_032-04w2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_032-04w2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_032-04w.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-04w2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_032-04w.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

//**********************************************************************//
// LIGHT 64 - 01 : Standard Ankh light source				//
//**********************************************************************//
textures/egyptsoc_sfx/lig_064-01b1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-01b1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-01b.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-01b1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-01b.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}


textures/egyptsoc_sfx/lig_064-01b2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-01b2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-01b.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-01b2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-01b.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-01r1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-01r1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-01r.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-01r1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-01r.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}


textures/egyptsoc_sfx/lig_064-01r2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-01r2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-01r.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-01r2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-01r.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-01y1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-01y1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-01y.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-01y1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-01y.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-01y2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-01y2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-01y.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-01y2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-01y.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-01w1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-01w1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-01w.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-01w1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-01w.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-01w2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-01w2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-01w.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-01w2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-01w.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

//**********************************************************************//
// LIGHT 64 - 02 : Standard Circle light source				//
//**********************************************************************//
textures/egyptsoc_sfx/lig_064-02b1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-02b1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-02b.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-02b1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-02b.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}


textures/egyptsoc_sfx/lig_064-02b2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-02b2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-02b.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-02b2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-02b.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-02r1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-02r1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-02r.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-02r1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-02r.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}


textures/egyptsoc_sfx/lig_064-02r2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-02r2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-02r.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-02r2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-02r.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-02y1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-02y1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-02y.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-02y1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-02y.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-02y2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-02y2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-02y.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-02y2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-02y.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-02w1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-02w1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-02w.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-02w1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-02w.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-02w2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-02w2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-02w.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-02w2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-02w.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

//**********************************************************************//
// LIGHT 64 - 04 : Circle template with cross bars across light source  //
//**********************************************************************//
textures/egyptsoc_sfx/lig_064-04b1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-04b1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-04b.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-04b1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-04b.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-04b2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-04b2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-04b.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-04b2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-04b.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-04r1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-04r1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-04r.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-04r1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-04r.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-04r2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-04r2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-04r.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-04r2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-04r.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-04y1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-04y1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-04y.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-04y1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-04y.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-04y2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-04y2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-04y.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-04y2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-04y.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-04w1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-04w1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-04w.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-04w1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-04w.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-04w2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-04w2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-04w.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-04w2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-04w.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

//**********************************************************************//
// LIGHT 64 - 05 : Standard Square light source				//
//**********************************************************************//
textures/egyptsoc_sfx/lig_064-05b1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-05b1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-05b.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-05b1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-05b.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-05b2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-05b2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-05b.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-05b2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-05b.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-05r1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-05r1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-05r.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-05r1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-05r.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-05r2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-05r2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-05r.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-05r2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-05r.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-05y1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-05y1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-05y.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-05y1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-05y.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-05y2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-05y2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-05y.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-05y2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-05y.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-05w1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-05w1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-05w.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-05w1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-05w.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-05w2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-05w2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-05w.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-05w2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-05w.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

//**********************************************************************//
// LIGHT 64 - 06 : Triangle light source				//
//**********************************************************************//
textures/egyptsoc_sfx/lig_064-06b1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-06b1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-06b.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-06b1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-06b.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-06b2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-06b2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-06b.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-06b2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-06b.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-06r1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-06r1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-06r.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-06r1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-06r.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-06r2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-06r2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-06r.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-06r2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-06r.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-06y1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-06y1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-06y.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-06y1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-06y.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-06y2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-06y2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-06y.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-06y2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-06y.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-06w1-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-06w1.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-06w.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-06w1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-06w.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/lig_064-06w2-2k
{
	qer_editorimage textures/egyptsoc_sfx/lig_064-06w2.tga
	q3map_lightimage textures/egyptsoc_sfx/lig_064-06w.blend.tga
	q3map_surfacelight 2000
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-06w2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/lig_064-06w.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

//**********************************************************************//
// LIGHT BAND 64 - 01 : Single glowing stripe 256x64			//
//**********************************************************************//
textures/egyptsoc_sfx/lig_b064-01a
{
	qer_editorimage textures/egyptsoc_sfx/lig_b064-01a.tga
	{
		map textures/egyptsoc_sfx/lig_b064-01a.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_b064-01b.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

textures/egyptsoc_sfx/lig_b064-01b
{
	qer_editorimage textures/egyptsoc_sfx/lig_b064-01b.tga
	{
		map textures/egyptsoc_sfx/lig_b064-01b.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_b064-01b.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

textures/egyptsoc_sfx/lig_b064-01c
{
	qer_editorimage textures/egyptsoc_sfx/lig_b064-01c.tga
	{
		map textures/egyptsoc_sfx/lig_b064-01c.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_b064-01y.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

textures/egyptsoc_sfx/lig_b064-01d
{
	qer_editorimage textures/egyptsoc_sfx/lig_b064-01d.tga
	{
		map textures/egyptsoc_sfx/lig_b064-01d.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_b064-01y.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

textures/egyptsoc_sfx/lig_b064-01e
{
	qer_editorimage textures/egyptsoc_sfx/lig_b064-01e.tga
	{
		map textures/egyptsoc_sfx/lig_b064-01e.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_b064-01r.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

textures/egyptsoc_sfx/lig_b064-01f
{
	qer_editorimage textures/egyptsoc_sfx/lig_b064-01f.tga
	{
		map textures/egyptsoc_sfx/lig_b064-01f.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_b064-01r.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

//**********************************************************************//
// LIGHT BAND 64 - 02 : ZigZag glowing stripe 256x64			//
//**********************************************************************//
textures/egyptsoc_sfx/lig_b064-02a
{
	qer_editorimage textures/egyptsoc_sfx/lig_b064-02a.tga
	{
		map textures/egyptsoc_sfx/lig_b064-02a.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_b064-02b.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

textures/egyptsoc_sfx/lig_b064-02b
{
	qer_editorimage textures/egyptsoc_sfx/lig_b064-02b.tga
	{
		map textures/egyptsoc_sfx/lig_b064-02b.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_b064-02b.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

textures/egyptsoc_sfx/lig_b064-02c
{
	qer_editorimage textures/egyptsoc_sfx/lig_b064-02c.tga
	{
		map textures/egyptsoc_sfx/lig_b064-02c.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_b064-02y.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

textures/egyptsoc_sfx/lig_b064-02d
{
	qer_editorimage textures/egyptsoc_sfx/lig_b064-02d.tga
	{
		map textures/egyptsoc_sfx/lig_b064-02d.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_b064-02y.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

textures/egyptsoc_sfx/lig_b064-02e
{
	qer_editorimage textures/egyptsoc_sfx/lig_b064-02e.tga
	{
		map textures/egyptsoc_sfx/lig_b064-02e.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_b064-02r.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

textures/egyptsoc_sfx/lig_b064-02f
{
	qer_editorimage textures/egyptsoc_sfx/lig_b064-02f.tga
	{
		map textures/egyptsoc_sfx/lig_b064-02f.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_b064-02r.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

//**********************************************************************//
// LIGHT BAND 64 - 03 : Triangle glowing pattern 256x64			//
//**********************************************************************//
textures/egyptsoc_sfx/lig_b064-03a
{
	qer_editorimage textures/egyptsoc_sfx/lig_b064-03a.tga
	{
		map textures/egyptsoc_sfx/lig_b064-03a.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_b064-03b.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

textures/egyptsoc_sfx/lig_b064-03b
{
	qer_editorimage textures/egyptsoc_sfx/lig_b064-03b.tga
	{
		map textures/egyptsoc_sfx/lig_b064-03b.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_b064-03b.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

textures/egyptsoc_sfx/lig_b064-03c
{
	qer_editorimage textures/egyptsoc_sfx/lig_b064-03c.tga
	{
		map textures/egyptsoc_sfx/lig_b064-03c.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_b064-03y.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

textures/egyptsoc_sfx/lig_b064-03d
{
	qer_editorimage textures/egyptsoc_sfx/lig_b064-03d.tga
	{
		map textures/egyptsoc_sfx/lig_b064-03d.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_b064-03y.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

textures/egyptsoc_sfx/lig_b064-03e
{
	qer_editorimage textures/egyptsoc_sfx/lig_b064-03e.tga
	{
		map textures/egyptsoc_sfx/lig_b064-03e.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_b064-03r.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

textures/egyptsoc_sfx/lig_b064-03f
{
	qer_editorimage textures/egyptsoc_sfx/lig_b064-03f.tga
	{
		map textures/egyptsoc_sfx/lig_b064-03f.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_b064-03r.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

//**********************************************************************//
// BLUE LIGHT 192 - 01 : 2 bar with additional border 96x192		//
//**********************************************************************//
textures/egyptsoc_sfx/lig_v192-01ba
{       
	qer_editorimage textures/egyptsoc_sfx/lig_v192-01ba.tga
	{
		map textures/egyptsoc_sfx/lig_v192-01ba.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_v192-01b.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

textures/egyptsoc_sfx/lig_v192-01bb
{       
	qer_editorimage textures/egyptsoc_sfx/lig_v192-01bb.tga
	{
		map textures/egyptsoc_sfx/lig_v192-01bb.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_v192-01b.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

//**********************************************************************//
// YELLOW LIGHT 192 - 01 : 2 bar with additional border 96x192		//
//**********************************************************************//
textures/egyptsoc_sfx/lig_v192-01ya
{       
	qer_editorimage textures/egyptsoc_sfx/lig_v192-01ya.tga
	{
		map textures/egyptsoc_sfx/lig_v192-01ya.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_v192-01y.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

textures/egyptsoc_sfx/lig_v192-01yb
{       
	qer_editorimage textures/egyptsoc_sfx/lig_v192-01yb.tga
	{
		map textures/egyptsoc_sfx/lig_v192-01yb.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_v192-01y.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

//**********************************************************************//
// WHITE LIGHT 192 - 01 : 2 bar with additional border 96x192		//
//**********************************************************************//
textures/egyptsoc_sfx/lig_v192-01wa
{       
	qer_editorimage textures/egyptsoc_sfx/lig_v192-01wa.tga
	{
		map textures/egyptsoc_sfx/lig_v192-01wa.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_v192-01w.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

textures/egyptsoc_sfx/lig_v192-01wb
{       
	qer_editorimage textures/egyptsoc_sfx/lig_v192-01wb.tga
	{
		map textures/egyptsoc_sfx/lig_v192-01wb.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/egyptsoc_sfx/lig_v192-01w.blend.tga
		blendfunc add	
                rgbgen wave sin .5 .3 0 1
	}
}

//**********************************************************************//
//									//
// FLOOR 1A								//
//									//
// Weapon Markers 1 - 2 rot swirls + FAST glowing symbols		//
//**********************************************************************//
textures/egyptsoc_sfx/wmblue_floor1a
{
	qer_editorimage textures/egyptsoc_floor/jumppad1ab.tga
	q3map_lightimage textures/egyptsoc_floor/jumppad1b.blend.tga	
	q3map_surfacelight 100
	{
		map textures/ctf/blue_telep.tga
                tcmod rotate 180
                tcMod stretch sin .8 0.1 0 .5
	}
	{
		map textures/ctf/blue_telep2.tga
		blendfunc ADD
                tcmod rotate 45
                tcMod stretch sin .8 0.1 0 .5
	}
	{
		map textures/egyptsoc_floor/jumppad1ab.tga
                blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
	{
		map textures/egyptsoc_floor/jumppad1b.blend.tga
		blendfunc ADD
		rgbgen wave sin .9 .1 0 5
	}
}

textures/egyptsoc_sfx/wmgold_floor1a
{
	qer_editorimage textures/egyptsoc_floor/jumppad1ag.tga
	q3map_lightimage textures/egyptsoc_floor/jumppad1g.blend.tga	
	q3map_surfacelight 100
	{
		map textures/egyptsoc_sfx/gold_telep.tga
                tcmod rotate 180
                tcMod stretch sin .8 0.1 0 .5
	}
	{
		map textures/egyptsoc_sfx/gold_telep2.tga
		blendfunc ADD
                tcmod rotate 45
                tcMod stretch sin .8 0.1 0 .5
	}
	{
		map textures/egyptsoc_floor/jumppad1ag.tga
                blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
	{
		map textures/egyptsoc_floor/jumppad1g.blend.tga
		blendfunc ADD
		rgbgen wave sin .9 .1 0 5
	}
}

textures/egyptsoc_sfx/wmred_floor1a
{
	qer_editorimage textures/egyptsoc_floor/jumppad1ar.tga
	q3map_lightimage textures/egyptsoc_floor/jumppad1r.blend.tga	
	q3map_surfacelight 100
	{
		map textures/ctf/red_telep.tga
                tcmod rotate 180
                tcMod stretch sin .8 0.1 0 .5
	}
	{
		map textures/ctf/red_telep2.tga
		blendfunc ADD
                tcmod rotate 45
                tcMod stretch sin .8 0.1 0 .5
	}
	{
		map textures/egyptsoc_floor/jumppad1ar.tga
                blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
	{
		map textures/egyptsoc_floor/jumppad1r.blend.tga
		blendfunc ADD
		rgbgen wave sin .9 .1 0 5
	}
}

//**********************************************************************//
//									//
// FLOOR 1B								//
//									//
// Weapon Markers 1 - 2 rot swirls + FAST glowing symbols		//
//**********************************************************************//
textures/egyptsoc_sfx/wmblue_floor1b
{
	qer_editorimage textures/egyptsoc_floor/jumppad1bb.tga
	q3map_lightimage textures/egyptsoc_floor/jumppad1b.blend.tga	
	q3map_surfacelight 100
	{
		map textures/ctf/blue_telep.tga
                tcmod rotate 180
                tcMod stretch sin .8 0.1 0 .5
	}
	{
		map textures/ctf/blue_telep2.tga
		blendfunc ADD
                tcmod rotate 45
                tcMod stretch sin .8 0.1 0 .5
	}
	{
		map textures/egyptsoc_floor/jumppad1bb.tga
                blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
	{
		map textures/egyptsoc_floor/jumppad1b.blend.tga
		blendfunc ADD
		rgbgen wave sin .9 .1 0 5
	}
}

textures/egyptsoc_sfx/wmgold_floor1b
{
	qer_editorimage textures/egyptsoc_floor/jumppad1bg.tga
	q3map_lightimage textures/egyptsoc_floor/jumppad1g.blend.tga	
	q3map_surfacelight 100
	{
		map textures/egyptsoc_sfx/gold_telep.tga
                tcmod rotate 180
                tcMod stretch sin .8 0.1 0 .5
	}
	{
		map textures/egyptsoc_sfx/gold_telep2.tga
		blendfunc ADD
                tcmod rotate 45
                tcMod stretch sin .8 0.1 0 .5
	}
	{
		map textures/egyptsoc_floor/jumppad1bg.tga
                blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
	{
		map textures/egyptsoc_floor/jumppad1g.blend.tga
		blendfunc ADD
		rgbgen wave sin .9 .1 0 5
	}
}

textures/egyptsoc_sfx/wmred_floor1b
{
	qer_editorimage textures/egyptsoc_floor/jumppad1br.tga
	q3map_lightimage textures/egyptsoc_floor/jumppad1r.blend.tga	
	q3map_surfacelight 100
	{
		map textures/ctf/red_telep.tga
                tcmod rotate 180
                tcMod stretch sin .8 0.1 0 .5
	}
	{
		map textures/ctf/red_telep2.tga
		blendfunc ADD
                tcmod rotate 45
                tcMod stretch sin .8 0.1 0 .5
	}
	{
		map textures/egyptsoc_floor/jumppad1br.tga
                blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
	{
		map textures/egyptsoc_floor/jumppad1r.blend.tga
		blendfunc ADD
		rgbgen wave sin .9 .1 0 5
	}
}

//**********************************************************************//
//									//
// FLOOR 1A								//
//									//
// Jump Pads 1 - 1 rot swirl + 1 jumppad stretch + SLOW glowing symbols //
//**********************************************************************//
textures/egyptsoc_sfx/jpblue_floor1a
{
	qer_editorimage textures/egyptsoc_floor/jumppad1ab.tga
	q3map_lightimage textures/egyptsoc_floor/jumppad1b.blend.tga	
	q3map_surfacelight 400
	{
		map textures/ctf/blue_telep.tga
                tcmod rotate 180
                tcMod stretch sin .8 0.1 0 .5
	}
	{
		clampmap textures/ctf/jumppadsmall_b.tga
		blendfunc ADD
		tcMod stretch sin 0.95 .7 0 1.25
		rgbGen wave square .5 .5 .25 1.25
		rgbgen identity
	}
	{
		map textures/egyptsoc_floor/jumppad1ab.tga
                blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
	{
		map textures/egyptsoc_floor/jumppad1b.blend.tga
		blendfunc ADD
		rgbgen wave sin .8 .2 0 1
	}
}

textures/egyptsoc_sfx/jpgold_floor1a
{
	qer_editorimage textures/egyptsoc_floor/jumppad1ag.tga
	q3map_lightimage textures/egyptsoc_floor/jumppad1g.blend.tga	
	q3map_surfacelight 400
	{
		map textures/egyptsoc_sfx/gold_telep.tga
                tcmod rotate 180
                tcMod stretch sin .8 0.1 0 .5
	}
	{
		clampmap textures/egyptsoc_sfx/jumppadsmall_g.tga
		blendfunc ADD
		tcMod stretch sin 0.95 .7 0 1.25
		rgbGen wave square .5 .5 .25 1.25
		rgbgen identity
	}
	{
		map textures/egyptsoc_floor/jumppad1ag.tga
                blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
	{
		map textures/egyptsoc_floor/jumppad1g.blend.tga
		blendfunc ADD
		rgbgen wave sin .8 .2 0 1
	}
}

textures/egyptsoc_sfx/jpred_floor1a
{
	qer_editorimage textures/egyptsoc_floor/jumppad1ar.tga
	q3map_lightimage textures/egyptsoc_floor/jumppad1r.blend.tga	
	q3map_surfacelight 400
	{
		map textures/ctf/red_telep.tga
                tcmod rotate 180
                tcMod stretch sin .8 0.1 0 .5
	}
	{
		clampmap textures/ctf/jumppadsmall_r.tga
		blendfunc ADD
		tcMod stretch sin 0.95 .7 0 1.25
		rgbGen wave square .5 .5 .25 1.25
		rgbgen identity
	}
	{
		map textures/egyptsoc_floor/jumppad1ar.tga
                blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
	{
		map textures/egyptsoc_floor/jumppad1r.blend.tga
		blendfunc ADD
		rgbgen wave sin .8 .2 0 1
	}
}

//**********************************************************************//
//									//
// FLOOR 1B								//
//									//
// Jump Pads 1 - 1 rot swirl + 1 jumppad stretch + SLOW glowing symbols //
//**********************************************************************//
textures/egyptsoc_sfx/jpblue_floor1b
{
	qer_editorimage textures/egyptsoc_floor/jumppad1bb.tga
	q3map_lightimage textures/egyptsoc_floor/jumppad1b.blend.tga	
	q3map_surfacelight 400
	{
		map textures/ctf/blue_telep.tga
                tcmod rotate 180
                tcMod stretch sin .8 0.1 0 .5
	}
	{
		clampmap textures/ctf/jumppadsmall_b.tga
		blendfunc ADD
		tcMod stretch sin 0.95 .7 0 1.25
		rgbGen wave square .5 .5 .25 1.25
		rgbgen identity
	}
	{
		map textures/egyptsoc_floor/jumppad1bb.tga
                blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
	{
		map textures/egyptsoc_floor/jumppad1b.blend.tga
		blendfunc ADD
		rgbgen wave sin .8 .2 0 1
	}
}

textures/egyptsoc_sfx/jpgold_floor1b
{
	qer_editorimage textures/egyptsoc_floor/jumppad1bg.tga
	q3map_lightimage textures/egyptsoc_floor/jumppad1g.blend.tga	
	q3map_surfacelight 400
	{
		map textures/egyptsoc_sfx/gold_telep.tga
                tcmod rotate 180
                tcMod stretch sin .8 0.1 0 .5
	}
	{
		clampmap textures/egyptsoc_sfx/jumppadsmall_g.tga
		blendfunc ADD
		tcMod stretch sin 0.95 .7 0 1.25
		rgbGen wave square .5 .5 .25 1.25
		rgbgen identity
	}
	{
		map textures/egyptsoc_floor/jumppad1bg.tga
                blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
	{
		map textures/egyptsoc_floor/jumppad1g.blend.tga
		blendfunc ADD
		rgbgen wave sin .8 .2 0 1
	}
}

textures/egyptsoc_sfx/jpred_floor1b
{
	qer_editorimage textures/egyptsoc_floor/jumppad1br.tga
	q3map_lightimage textures/egyptsoc_floor/jumppad1r.blend.tga	
	q3map_surfacelight 400
	{
		map textures/ctf/red_telep.tga
                tcmod rotate 180
                tcMod stretch sin .8 0.1 0 .5
	}
	{
		clampmap textures/ctf/jumppadsmall_r.tga
		blendfunc ADD
		tcMod stretch sin 0.95 .7 0 1.25
		rgbGen wave square .5 .5 .25 1.25
		rgbgen identity
	}
	{
		map textures/egyptsoc_floor/jumppad1br.tga
                blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		rgbGen identity
	}
	{
		map textures/egyptsoc_floor/jumppad1r.blend.tga
		blendfunc ADD
		rgbgen wave sin .8 .2 0 1
	}
}

//**********************************************************************//
// S128-01 : Weapon Markers with glowing central area's			//
//**********************************************************************//
textures/egyptsoc_sfx/s128-01wc
{
	qer_editorimage textures/egyptsoc_trim/s128-01c.tga
	q3map_lightimage textures/egyptsoc_sfx/s128-01b.blend.tga
	light 1
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_trim/s128-01c.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/s128-01b.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/s128-01wd
{
	qer_editorimage textures/egyptsoc_trim/s128-01d.tga
	q3map_lightimage textures/egyptsoc_sfx/s128-01b.blend.tga
	light 1
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_trim/s128-01d.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/s128-01b.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/s128-01we
{
	qer_editorimage textures/egyptsoc_trim/s128-01e.tga
	q3map_lightimage textures/egyptsoc_sfx/s128-01g.blend.tga
	light 1
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_trim/s128-01e.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/s128-01g.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/s128-01wcr
{
	qer_editorimage textures/egyptsoc_trimd/s128-01cr.tga
	q3map_lightimage textures/egyptsoc_sfx/s128-01r.blend.tga
	light 1
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_trimd/s128-01cr.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/s128-01r.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

textures/egyptsoc_sfx/s128-01wdr
{
	qer_editorimage textures/egyptsoc_trimd/s128-01dr.tga
	q3map_lightimage textures/egyptsoc_sfx/s128-01r.blend.tga
	light 1
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/egyptsoc_trimd/s128-01dr.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/egyptsoc_sfx/s128-01r.blend.tga
		blendfunc GL_ONE GL_ONE
		rgbGen wave sin .3 .1 0 0.5
	}
}

//**********************************************************************//
// GRATE : 128x128 designs						//
//**********************************************************************//
// grate1a  grate1b  - square 32x32 design
// grate2a  grate2b  - small rectangle design
// grate3a  grate3b  - ring 32x32 design
// grate4a  grate4b  - detail version of ring design
//
textures/egyptsoc_floor/grate1a
{
	surfaceparm	metalsteps		
	cull none
	{
		map textures/egyptsoc_floor/grate1a.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		depthWrite
		rgbGen identity
	}
	{
		map $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		depthFunc equal
		rgbGen identity
	}
}

textures/egyptsoc_floor/grate1b
{
	surfaceparm	metalsteps		
	cull none
	{
		map textures/egyptsoc_floor/grate1b.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		depthWrite
		rgbGen identity
	}
	{
		map $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		depthFunc equal
		rgbGen identity
	}
}

textures/egyptsoc_floor/grate2a
{
	surfaceparm	metalsteps		
	cull none
	{
		map textures/egyptsoc_floor/grate2a.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		depthWrite
		rgbGen identity
	}
	{
		map $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		depthFunc equal
		rgbGen identity
	}
}

textures/egyptsoc_floor/grate2b
{
	surfaceparm	metalsteps		
	cull none
	{
		map textures/egyptsoc_floor/grate2b.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		depthWrite
		rgbGen identity
	}
	{
		map $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		depthFunc equal
		rgbGen identity
	}
}

textures/egyptsoc_floor/grate3a
{
	surfaceparm	metalsteps		
	cull none
	{
		map textures/egyptsoc_floor/grate3a.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		depthWrite
		rgbGen identity
	}
	{
		map $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		depthFunc equal
		rgbGen identity
	}
}

textures/egyptsoc_floor/grate3b
{
	surfaceparm	metalsteps		
	cull none
	{
		map textures/egyptsoc_floor/grate3b.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		depthWrite
		rgbGen identity
	}
	{
		map $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		depthFunc equal
		rgbGen identity
	}
}

textures/egyptsoc_floor/grate4a
{
	surfaceparm	metalsteps		
	cull none
	{
		map textures/egyptsoc_floor/grate4a.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		depthWrite
		rgbGen identity
	}
	{
		map $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		depthFunc equal
		rgbGen identity
	}
}

textures/egyptsoc_floor/grate4b
{
	surfaceparm	metalsteps		
	cull none
	{
		map textures/egyptsoc_floor/grate4b.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		depthWrite
		rgbGen identity
	}
	{
		map $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		depthFunc equal
		rgbGen identity
	}
}
