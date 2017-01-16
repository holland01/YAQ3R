## Subjects to be mindful of

	- key slot indexing

	- If G_UNSPECIFIED is passed to GBindTexture, then a dummy white texture
	is bound by default.

	- samplers do matter; for now it's best to leave the API functions
	available so they can be called. At the moment they're nothing more than
	just blank useless IDs, but that will change if mipmaps are introduced

	- did the image row reversal for stb loaded images that's required
	(due to OpenGL's different coordinate system conventions) get skipped in
	the content pipeline loader rewrite? I don't see it in
	`GImageLoadFromMemory`.

	

### async_image_io module

	- the alignment used here (line 79):
		`std::vector< uint8_t > imageData( Align( width * height * bpp ), 0 );`
		might actually be what's causing black splotches to show. Note that
		this is also done in `file_t::ReadImage` (worker/file_traverse.cxx)

	- each instance should initialize an atlas instance so it can hold it
	and keep track of it

### GU_SetupTexParams

	- `texHandle` refers to the atlas itself

	- `textureIndex` is the image to be sampled from (which is a part of
	`texHandle`)

	- passing `textureIndex < 0` will just release the last bound texture
	and then cause it to return.

	- `offset` param refers to the texture slot to bind the whole atlas to.
		if it's -1, then no actual sampler is used... (wtf?)

	- the sampling method is implicit in the fragment shader - the current
	implementation of this function relies on relevant metadata that's obtained
	through the call to `GTextureImageShaderParams( texHandle, textureIndex );`

### GU_LoadMainTextures

	- Textures sampled in the main shader are loaded here. They're
	 key mapped. the main shader isn't dynamically generated
 	from the .shader files.

### BSPRenderer::DrawEffectPass

	- in the stage loop, `texDims` is used for tcMod scroll
		- not sure why 64 is set though. I feel like that's not right

	- `GReleaseTexture` is called - the offset parameter has a default of 0,
	so it seems like when effect shaders are used that slot is
	just bound to by default. This makes sense, considering that every
	image or lightmap used in an effect shader is assigned its own
	specific render pass.
