//***************************************************************
// multiplant.shader	Todd Gantzler/Obsidian		25.02.05
// Originally by Todd Gantzer, rewritten by Obsidian for Q3Map2
// support and various bug fixes and optimizations. Models are
// vertex lit by default.
//***************************************************************


models/mapobjects/multiplant/fern
{
	cull none

	surfaceparm alphashadow
	surfaceparm pointlight
	surfaceparm trans

	qer_editorImage models/mapobjects/multiplant/fern.tga
	qer_alphaFunc gequal 0.5

	{
		map models/mapobjects/multiplant/fern.tga
		rgbGen vertex
		alphaGen const 1.0
	//	rgbGen exactVertex
		alphaFunc GE128
		depthWrite
	}
}


models/mapobjects/multiplant/bannanaleaf
{	
	cull none

	surfaceparm alphashadow
	surfaceparm pointlight
	surfaceparm trans

	qer_editorImage models/mapobjects/multiplant/bannanaleaf.tga
	qer_alphaFunc gequal 0.5

	{
		map models/mapobjects/multiplant/bannanaleaf.tga
		rgbGen vertex
		alphaGen const 1.0
	//	rgbGen exactVertex
		alphaFunc GE128
		depthWrite
	}
}


models/mapobjects/multiplant/batleaf
{
	cull none

	surfaceparm alphashadow
	surfaceparm pointlight
	surfaceparm trans

	qer_editorImage models/mapobjects/multiplant/batleaf.tga
	qer_alphaFunc gequal 0.5

	{
		map models/mapobjects/multiplant/batleaf.tga
		rgbGen vertex
		alphaGen const 1.0
	//	rgbGen exactVertex
		alphaFunc GE128
		depthWrite
	}
}


models/mapobjects/multiplant/batleaf2
{
	cull none
	deformVertexes wave 100 sin 3 5 0.1 0.1

	surfaceparm alphashadow
	surfaceparm pointlight
	surfaceparm trans

	qer_editorImage models/mapobjects/multiplant/batleaf2.tga
	qer_alphaFunc gequal 0.5

	{
		map models/mapobjects/multiplant/batleaf2.tga
		rgbGen vertex
		alphaGen const 1.0
	//	rgbGen exactVertex
		alphaFunc GE128
		depthWrite
	}
	{
		map models/mapobjects/multiplant/batleaf2veins.tga
		blendFunc blend
		rgbGen wave sin 0 1 0 .33
		depthFunc equal
	}
}


models/mapobjects/multiplant/leaf1
{
	cull none

	surfaceparm alphashadow
	surfaceparm pointlight
	surfaceparm trans

	qer_editorImage models/mapobjects/multiplant/leaf1.tga
	qer_alphaFunc gequal 0.5

	{
		map models/mapobjects/multiplant/leaf1.tga
		rgbGen vertex
		alphaGen const 1.0
	//	rgbGen exactVertex
		alphaFunc GE128
		depthWrite
	}
}


models/mapobjects/multiplant/leaf2
{
	cull none

	surfaceparm alphashadow
	surfaceparm pointlight
	surfaceparm trans

	qer_editorImage models/mapobjects/multiplant/leaf2.tga
	qer_alphaFunc gequal 0.5

	{
		map models/mapobjects/multiplant/leaf2.tga
		rgbGen vertex
		alphaGen const 1.0
	//	rgbGen exactVertex
		alphaFunc GE128
		depthWrite
	}
}


models/mapobjects/multiplant/leaf3
{
	cull none

	surfaceparm alphashadow
	surfaceparm pointlight
	surfaceparm trans

	qer_editorImage models/mapobjects/multiplant/leaf3.tga
	qer_alphaFunc gequal 0.5

	{
		map models/mapobjects/multiplant/leaf3.tga
		rgbGen vertex
		alphaGen const 1.0
	//	rgbGen exactVertex
		alphaFunc GE128
		depthWrite
	}
}


models/mapobjects/multiplant/pleaf1
{
	cull none
	
	surfaceparm alphashadow
	surfaceparm pointlight
	surfaceparm trans

	qer_editorImage models/mapobjects/multiplant/pleaf1.tga
	qer_alphaFunc gequal 0.5

	{
		map models/mapobjects/multiplant/pleaf1.tga
		rgbGen vertex
		alphaGen const 1.0
	//	rgbGen exactVertex
		alphaFunc GE128
		depthWrite
	}
}


models/mapobjects/multiplant/pleaf2
{
	cull none

	surfaceparm alphashadow
	surfaceparm pointlight
	surfaceparm trans
	
	qer_editorImage models/mapobjects/multiplant/pleaf2.tga
	qer_alphaFunc gequal 0.5

	{
		map models/mapobjects/multiplant/pleaf2.tga
		rgbGen vertex
		alphaGen const 1.0
	//	rgbGen exactVertex
		alphaFunc GE128
		depthWrite
	}
}


models/mapobjects/multiplant/pleaf3
{
	cull none

	surfaceparm alphashadow
	surfaceparm pointlight
	surfaceparm trans

	qer_editorImage models/mapobjects/multiplant/pleaf3.tga
	qer_alphaFunc gequal 0.5

	{
		map models/mapobjects/multiplant/pleaf3.tga
		rgbGen vertex
		alphaGen const 1.0
	//	rgbGen exactVertex
		alphaFunc GE128
		depthWrite
	}
}


models/mapobjects/multiplant/palmfrond
{
	cull none

	surfaceparm alphashadow
	surfaceparm pointlight
	surfaceparm trans

	qer_editorImage models/mapobjects/multiplant/palmfrond.tga
	qer_alphaFunc gequal 0.5

	{
		map models/mapobjects/multiplant/palmfrond.tga
		rgbGen vertex
		alphaGen const 1.0
	//	rgbGen exactVertex
		alphaFunc GE128
		depthWrite
	}
}


// Special dual purpose shader:
// Legacy maps will display an "invisible" shader on polygons.
// Newly compiled maps will use a "nodraw" shader and extra
// polys will not be drawn in game for improved performance.
models/mapobjects/multiplant/invisible
{
	surfaceparm nodraw
	surfaceparm nolightmap
	surfaceparm nonsolid	//forces nonsolid even with autoclip enabled
	surfaceparm trans

	qer_editorImage textures/common/nodraw
	qer_trans 0.5

	{
		map models/mapobjects/multiplant/invisible.tga
		rgbGen vertex
		alphaFunc GE128
		depthWrite
	}
}


//***************************************************************
// palms.shader		Todd Gantzler/Obsidian		25.02.05
// Originally by Todd Gantzer, rewritten by Obsidian for Q3Map2
// support and various bug fixes and optimizations. Models are
// vertex lit by default. Merged contents of palms.shader here.
//***************************************************************


models/mapobjects/palm2/trunk
{
	surfaceparm pointlight

	qer_editorImage models/mapobjects/palm2/trunk.tga

	{
		map models/mapobjects/palm2/trunk.tga
		rgbGen vertex
	}
}


models/mapobjects/palm1/palm1
{
	deformVertexes autosprite2
	cull none

	surfaceparm alphashadow
	surfaceparm pointlight
	surfaceparm trans

	qer_editorImage models/mapobjects/palm1/palm1.tga
	qer_alphaFunc gequal 0.5

	{
		map models/mapobjects/palm1/palm1.tga
		rgbGen vertex
		alphaGen const 1.0
	//	rgbGen exactVertex
		alphaFunc GE128
		depthWrite
	}
}


models/mapobjects/palm3/palmtop
{
	deformVertexes autosprite
	cull none

	surfaceparm alphashadow
	surfaceparm pointlight
	surfaceparm trans

	qer_editorImage models/mapobjects/palm3/palmtop.tga
	qer_alphaFunc gequal 0.5

	{
		map models/mapobjects/palm3/palmtop.tga
		rgbGen vertex
		alphaGen const 1.0
	//	rgbGen exactVertex
		alphaFunc GE128
		depthWrite
	}
}