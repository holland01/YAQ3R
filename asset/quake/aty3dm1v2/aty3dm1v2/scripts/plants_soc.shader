//**********************************************************************
//
//	plants_soc.shader for Q3 by Sims
//	Website : http://www.simonoc.com/
//
//**********************************************************************
//
// ======================================================================
// PLANTS
// ======================================================================
textures/plants_soc/leaf01a
{
	qer_editorimage textures/plants_soc/leaf01a.tga
	q3map_cloneShader textures/plants_soc/leaf01a_back

	q3map_vertexScale 1.25
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm alphashadow
	qer_alphafunc greater 0.5
	qer_trans 0.99

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf01a.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf01a.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/leaf01a_back
{
	qer_editorimage textures/plants_soc/leaf01a.tga

	q3map_invert
	q3map_vertexScale 3.5
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf01a.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf01a.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}

// ----------------------------------------------------------------------
textures/plants_soc/leaf01b
{
	qer_editorimage textures/plants_soc/leaf01b.tga
	q3map_cloneShader textures/plants_soc/leaf01b_back

	q3map_vertexScale 1.25
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm alphashadow
	qer_alphafunc greater 0.5
	qer_trans 0.99

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf01b.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf01b.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/leaf01b_back
{
	qer_editorimage textures/plants_soc/leaf01b.tga

	q3map_invert
	q3map_vertexScale 3.5
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf01b.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf01b.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/leaf01d
{
	qer_editorimage textures/plants_soc/leaf01d.tga
	q3map_cloneShader textures/plants_soc/leaf01b_back

	q3map_vertexScale 1.25
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm alphashadow
	qer_alphafunc greater 0.5
	qer_trans 0.99

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf01d.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf01d.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/leaf01d_back
{
	qer_editorimage textures/plants_soc/leaf01d.tga

	q3map_invert
	q3map_vertexScale 3.5
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf01d.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf01d.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/leaf02a
{
	qer_editorimage textures/plants_soc/leaf02a.tga
	q3map_cloneShader textures/plants_soc/leaf02a_back

	q3map_vertexScale 1.25
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm alphashadow
	qer_alphafunc greater 0.5
	qer_trans 0.99

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf02a.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf02a.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/leaf02a_back
{
	qer_editorimage textures/plants_soc/leaf02a.tga

	q3map_invert
	q3map_vertexScale 3.5
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf02a.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf02a.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/leaf02b
{
	qer_editorimage textures/plants_soc/leaf02b.tga
	q3map_cloneShader textures/plants_soc/leaf02b_back

	q3map_vertexScale 1.25
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm alphashadow
	qer_alphafunc greater 0.5
	qer_trans 0.99

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf02b.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf02b.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/leaf02b_back
{
	qer_editorimage textures/plants_soc/leaf02b.tga

	q3map_invert
	q3map_vertexScale 3.5
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf02b.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf02b.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/leaf02c
{
	qer_editorimage textures/plants_soc/leaf02c.tga
	q3map_cloneShader textures/plants_soc/leaf02c_back

	q3map_vertexScale 1.25
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm alphashadow
	qer_alphafunc greater 0.5
	qer_trans 0.99

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf02c.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf02c.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/leaf02c_back
{
	qer_editorimage textures/plants_soc/leaf02c.tga

	q3map_invert
	q3map_vertexScale 3.5
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf02c.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf02c.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/leaf02d
{
	qer_editorimage textures/plants_soc/leaf02d.tga
	q3map_cloneShader textures/plants_soc/leaf02d_back

	q3map_vertexScale 1.25
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm alphashadow
	qer_alphafunc greater 0.5
	qer_trans 0.99

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf02d.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf02d.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/leaf02d_back
{
	qer_editorimage textures/plants_soc/leaf02d.tga

	q3map_invert
	q3map_vertexScale 3.5
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf02d.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf02d.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/leaf03a
{
	qer_editorimage textures/plants_soc/leaf03a.tga
	q3map_cloneShader textures/plants_soc/leaf03a_back

	q3map_vertexScale 1.25
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm alphashadow
	qer_alphafunc greater 0.5
	qer_trans 0.99

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf03a.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf03a.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/leaf03a_back
{
	qer_editorimage textures/plants_soc/leaf03a.tga

	q3map_invert
	q3map_vertexScale 3.5
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf03a.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf03a.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/leaf03b
{
	qer_editorimage textures/plants_soc/leaf03b.tga
	q3map_cloneShader textures/plants_soc/leaf03b_back

	q3map_vertexScale 1.25
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm alphashadow
	qer_alphafunc greater 0.5
	qer_trans 0.99

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf03b.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf03b.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/leaf03b_back
{
	qer_editorimage textures/plants_soc/leaf03b.tga

	q3map_invert
	q3map_vertexScale 3.5
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf03b.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf03b.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/leaf03c
{
	qer_editorimage textures/plants_soc/leaf03c.tga
	q3map_cloneShader textures/plants_soc/leaf03c_back

	q3map_vertexScale 1.25
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm alphashadow
	qer_alphafunc greater 0.5
	qer_trans 0.99

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf03c.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf03c.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/leaf03c_back
{
	qer_editorimage textures/plants_soc/leaf03c.tga

	q3map_invert
	q3map_vertexScale 3.5
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf03c.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf03c.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/leaf04a
{
	qer_editorimage textures/plants_soc/leaf04a.tga
	q3map_cloneShader textures/plants_soc/leaf04a_back

	q3map_vertexScale 1.25
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm alphashadow
	qer_alphafunc greater 0.5
	qer_trans 0.99

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf04a.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf04a.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/leaf04a_back
{
	qer_editorimage textures/plants_soc/leaf04a.tga

	q3map_invert
	q3map_vertexScale 3.5
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf04a.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf04a.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/leaf04b
{
	qer_editorimage textures/plants_soc/leaf04b.tga
	q3map_cloneShader textures/plants_soc/leaf04b_back

	q3map_vertexScale 1.25
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm alphashadow
	qer_alphafunc greater 0.5
	qer_trans 0.99

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf04b.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf04b.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/leaf04b_back
{
	qer_editorimage textures/plants_soc/leaf04b.tga

	q3map_invert
	q3map_vertexScale 3.5
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf04b.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf04b.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/leaf04c
{
	qer_editorimage textures/plants_soc/leaf04c.tga
	q3map_cloneShader textures/plants_soc/leaf04c_back

	q3map_vertexScale 1.25
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm alphashadow
	qer_alphafunc greater 0.5
	qer_trans 0.99

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf04c.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf04c.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/leaf04c_back
{
	qer_editorimage textures/plants_soc/leaf04c.tga

	q3map_invert
	q3map_vertexScale 3.5
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf04c.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf04c.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/leaf04d
{
	qer_editorimage textures/plants_soc/leaf04d.tga
	q3map_cloneShader textures/plants_soc/leaf04d_back

	q3map_vertexScale 1.25
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm alphashadow
	qer_alphafunc greater 0.5
	qer_trans 0.99

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf04d.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf04d.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/leaf04d_back
{
	qer_editorimage textures/plants_soc/leaf04d.tga

	q3map_invert
	q3map_vertexScale 3.5
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf04d.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf04d.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/leaf05a
{
	qer_editorimage textures/plants_soc/leaf05a.tga
	q3map_cloneShader textures/plants_soc/leaf05a_back

	q3map_vertexScale 1.25
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm alphashadow
	qer_alphafunc greater 0.5
	qer_trans 0.99

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf05a.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf05a.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/leaf05a_back
{
	qer_editorimage textures/plants_soc/leaf05a.tga

	q3map_invert
	q3map_vertexScale 3.5
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

//	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/leaf05a.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/leaf05a.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ======================================================================
// GRASS
// ======================================================================
textures/plants_soc/grass01a
{
	qer_editorimage textures/plants_soc/grass01a.tga
	q3map_cloneShader textures/plants_soc/grass01a_back

	q3map_vertexScale 1.25
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm pointlight
	qer_trans 0.99

	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/grass01a.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/grass01a.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/grass01a_back
{
	qer_editorimage textures/plants_soc/grass01a.tga

	q3map_invert
	q3map_vertexScale 1.5
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm pointlight

	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/grass01a.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/grass01a.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/grass01d
{
	qer_editorimage textures/plants_soc/grass01d.tga
	q3map_cloneShader textures/plants_soc/grass01d_back

	q3map_vertexScale 1.25
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm pointlight
	qer_trans 0.99

	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/grass01d.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/grass01d.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/grass01d_back
{
	qer_editorimage textures/plants_soc/grass01d.tga

	q3map_invert
	q3map_vertexScale 1.5
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm pointlight

	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/grass01d.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/grass01d.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/grass01e
{
	qer_editorimage textures/plants_soc/grass01e.tga
	q3map_cloneShader textures/plants_soc/grass01e_back

	q3map_vertexScale 1.25
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm pointlight
	qer_trans 0.99

	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/grass01e.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/grass01e.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/grass01e_back
{
	qer_editorimage textures/plants_soc/grass01e.tga

	q3map_invert
	q3map_vertexScale 1.5
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm pointlight

	deformVertexes wave 16 sin 0 0.5 0 0.1
   {
		map textures/plants_soc/grass01e.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/grass01e.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ----------------------------------------------------------------------
// Used under the grass plant model
// ----------------------------------------------------------------------
textures/plants_soc/decal_plant1
{
	qer_editorimage textures/plants_soc/decal_plant1.tga
	q3map_bounceScale 0
   surfaceparm nonsolid
	surfaceparm trans
   surfaceparm nomarks
	surfaceparm nolightmap
   polygonOffset
   {
      map textures/plants_soc/decal_plant1.tga
      blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
      detail
   }
}

// ======================================================================
// VINE
// ======================================================================
textures/plants_soc/vine01
{
	qer_editorimage textures/plants_soc/vine01.tga
	q3map_cloneShader textures/plants_soc/vine01_back
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	qer_trans 0.99

	{
		map textures/plants_soc/vine01.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/vine01_back
{
	qer_editorimage textures/plants_soc/vine01.tga

	q3map_invert
	q3map_vertexScale 0.75
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

	deformVertexes wave 16 sin 0 0.5 0 0.1
	{
		map textures/plants_soc/vine01.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/vine02
{
	qer_editorimage textures/plants_soc/vine02.tga
	q3map_cloneShader textures/plants_soc/vine02_back
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	qer_trans 0.99

	{
		map textures/plants_soc/vine02.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/vine02_back
{
	qer_editorimage textures/plants_soc/vine02.tga

	q3map_invert
	q3map_vertexScale 0.75
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

	deformVertexes wave 16 sin 0 0.5 0 0.1
	{
		map textures/plants_soc/vine02.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/vine03
{
	qer_editorimage textures/plants_soc/vine03.tga
	q3map_cloneShader textures/plants_soc/vine03_back
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	qer_trans 0.99

	{
		map textures/plants_soc/vine03.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/vine03_back
{
	qer_editorimage textures/plants_soc/vine03.tga

	q3map_invert
	q3map_vertexScale 0.75
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

	deformVertexes wave 16 sin 0 0.5 0 0.1
	{
		map textures/plants_soc/vine03.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/vine04
{
	qer_editorimage textures/plants_soc/vine04.tga
	q3map_cloneShader textures/plants_soc/vine04_back
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	qer_trans 0.99

	{
		map textures/plants_soc/vine04.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/vine04_back
{
	qer_editorimage textures/plants_soc/vine04.tga

	q3map_invert
	q3map_vertexScale 0.75
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

	deformVertexes wave 16 sin 0 0.5 0 0.1
	{
		map textures/plants_soc/vine04.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/vine05
{
	qer_editorimage textures/plants_soc/vine05.tga
	q3map_cloneShader textures/plants_soc/vine05_back
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	qer_trans 0.99

	{
		map textures/plants_soc/vine05.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/vine05_back
{
	qer_editorimage textures/plants_soc/vine05.tga

	q3map_invert
	q3map_vertexScale 0.75
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

	deformVertexes wave 16 sin 0 0.5 0 0.1
	{
		map textures/plants_soc/vine05.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/vine06
{
	qer_editorimage textures/plants_soc/vine06.tga
	q3map_cloneShader textures/plants_soc/vine06_back
	q3map_vertexScale 1.25
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	qer_trans 0.99

	{
		map textures/plants_soc/vine06.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/vine06_back
{
	qer_editorimage textures/plants_soc/vine06.tga

	q3map_invert
	q3map_vertexScale 0.75
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

	deformVertexes wave 16 sin 0 0.5 0 0.1
	{
		map textures/plants_soc/vine06.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ======================================================================
// MUSHROOM
// ======================================================================
textures/plants_soc/mush_top01
{
	qer_editorimage textures/plants_soc/mush_top01.tga
	q3map_cloneShader textures/plants_soc/mush_back01
	q3map_vertexScale 1.25
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm alphashadow
	qer_trans 0.99

   {
		map textures/plants_soc/mush_top01.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/mush_top01.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}

// ----------------------------------------------------------------------
textures/plants_soc/mush_back01
{
	qer_editorimage textures/plants_soc/mush_back01.tga

	q3map_invert
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

   {
		map textures/plants_soc/mush_back01.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		map textures/plants_soc/mush_back01.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}

// ----------------------------------------------------------------------
textures/plants_soc/mush_stem01
{
	qer_editorimage textures/plants_soc/mush_stem01.tga
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks

	q3map_nonplanar
	q3map_shadeAngle 75

	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/plants_soc/mush_stem01.tga
		blendFunc GL_DST_COLOR GL_ZERO
		tcmod Scale 2 2
	}
}
// ======================================================================
// TREE
// ======================================================================
textures/plants_soc/tree01_bark
{
	qer_editorimage textures/plants_soc/tree01_bark.tga

	q3map_nonplanar
	q3map_shadeAngle 75

	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/plants_soc/tree01_bark.tga
		blendFunc GL_DST_COLOR GL_ZERO
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/tree01_stump
{
	qer_editorimage textures/plants_soc/tree01_stump.tga

	q3map_nonplanar
	q3map_shadeAngle 75

	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/plants_soc/tree01_stump.tga
		blendFunc GL_DST_COLOR GL_ZERO
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/tree01_leaves
{
	qer_editorimage textures/plants_soc/tree01_leaves.tga
	q3map_cloneShader textures/plants_soc/tree01_leavesback

	q3map_vertexScale 1.25
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm alphashadow
	qer_trans 0.99

	deformVertexes wave 16 sin 0 1 0 .2
	{
		map textures/plants_soc/tree01_leaves.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
textures/plants_soc/tree01_leavesback
{
	qer_editorimage textures/plants_soc/tree01_leaves.tga

	q3map_invert
	q3map_vertexScale 3.5
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nolightmap

	deformVertexes wave 16 sin 0 1 0 .2
	{
		map textures/plants_soc/tree01_leaves.tga
		alphaFunc GE128
		rgbGen vertex
		depthWrite
	}
}
// ======================================================================
// BLACK SKY
// ======================================================================
textures/plants_soc/sky_black
{
	qer_editorimage textures/plants_soc/black.jpg

	//red green blue intensity degrees elevation deviance samples
	q3map_sunExt 1 1 .93 125 270 50 2 32
	q3map_skyLight 125 6

	q3map_noFog
	q3map_globalTexture
	surfaceparm sky
	surfaceparm noimpact
	surfaceparm nolightmap
	nopicmip
	{
		map textures/plants_soc/black.tga
	}
}
// ======================================================================
// TERRAIN FLOOR BLEND (example)
// ======================================================================
textures/plants_soc/ter_mossgravel
{
	qer_editorimage textures/plants_soc/ter_mossgravel.tga
	q3map_nonplanar
	q3map_shadeangle 60
	q3map_tcGen ivector ( 128 0 0 ) ( 0 128 0 )
	q3map_alphaMod dotproduct2 ( 0.0 0.0 0.75 )
	{
		// Primary
		map textures/plants_soc/ter_moss2.tga
		rgbGen identity
	}
	{
		// Secondary
		map textures/plants_soc/ter_gravel1.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaFunc GE128		// Hard blend
		rgbGen identity
		alphaGen vertex
	}
	{
		map $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
}
// ======================================================================
// ROCK WALL + BLEND (example)
// ======================================================================
textures/plants_soc/rock_grey2
{
	qer_editorimage textures/plants_soc/rock_grey2.tga
	
	q3map_nonplanar
	q3map_shadeAngle 75
	
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/plants_soc/rock_grey2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/rock_grey2vine
{
	qer_editorimage textures/plants_soc/rock_grey2vine.tga
	
	q3map_nonplanar
	q3map_shadeAngle 75
	
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/plants_soc/rock_grey2vine.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
}
// ----------------------------------------------------------------------
textures/plants_soc/rock_grey2blend
{
	qer_editorimage textures/plants_soc/rock_grey2vine.tga
	
	q3map_nonplanar
	q3map_shadeAngle 75
	
	{
		// Primary
		map textures/plants_soc/rock_grey2.tga
		rgbGen identity
	}
	{
		// Secondary
		map textures/plants_soc/rock_grey2vine.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaFunc GE128
		rgbGen identity
		alphaGen vertex
	}
	{
		map $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
}
// ======================================================================
// ALPHA BRUSHES
// ======================================================================
textures/plants_soc/alpha_000
{
	qer_editorimage textures/plants_soc/alpha_000.tga
	q3map_alphaMod volume
	q3map_alphaMod set 0
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm trans
	qer_trans 0.7
}
textures/plants_soc/alpha_100
{
	qer_editorimage textures/plants_soc/alpha_100.tga
	q3map_alphaMod volume
	q3map_alphaMod set 1
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm trans
	qer_trans 0.7
}
