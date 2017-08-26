[feature/effect_shaders]
* Review the section in the Q3 Shader Manual for the tcModScale effect; it should shed some light on issues happening with the
renderer's current implementation.

[feature/effect_shaders]
Fix lava rising effect.

[bugfix/atlas_seam_filtering]
There's a lot of material worth reviewing; the details behind
perspective-correct texture mapping are important, here.

Understand at the very least the `drawAffineTexturedPolygon()` and
`drawPerspectiveTexturedPolygon()` functions found
[here](http://www.flipcode.com/archives/High_Speed_Software_Rendering.shtml).

It's very likely that an understanding of this information will provide
enough insight to fix this. Obviously trivial solutions aren't working,
likely due to the nature of the Atlas itself and how it's being constructed.

The BSP algorithm used to create the atlas is efficient in terms of the
space it allocates for each image. That said, there's almost always
going to be pockets of empty space which aren't used. They should be sparse,
though.

A quick and dirty hack that might work for this would be to sample a fraction
of the image and add that as a border around it. Right edges would "wrap",
providing the beginning of the left edge, for example. This should
provide enough room to ensure that the interpolation doesn't wind up crossing
into the next region.

What's somewhat irritating is that the modulo should technically already
be taking care of this: the texcoord which is passed to the fragment shader
is already interpolated across the triangle, and still within the expected
[0, 1] range, proportional to the polygon's vertices (i.e., no atlas
calculations are provided). So, even modding the value against 0.99
before applying the atlas transform, and THEN sampling causes this to happen.

Also tried the following,

```
static INLINE void WriteTexture(
		std::vector< std::string >& fragmentSrc,
		const shaderStage_t& stage,
		const char* discardPredicate )
{
	std::string sampleTextureExpr;

	if ( stage.mapCmd == MAP_CMD_CLAMPMAP )
	{
		fragmentSrc.push_back( "\tst = clamp( applyTransform( st )," \
			 "imageTransform.xy + vec2( 0.1 ), applyTransform( vec2( 0.9 ) ) );" );
	}
	else
	{
		fragmentSrc.push_back(
			"\tst = applyTransform( vec2( 0.1 ) + mod( st, vec2( 0.9 ) ) );" );
	}
```

Note how the st coords are clamped between [0.1, 0.9] before being transformed.
This doesn't solve much at all, and even produces artifacts on the skybox
when nearest filtering is used.

A potential hack would be applying smaller biases, something like
[0.05, 0.95] to scrolling textures.

The scaling shouldn't really be affecting this, because the transform is affine
and hence shouldn't cause the result to have a poor proportion relative
to the initially provided coordinates _unless_ precision is a major
factor which, technically, it could be.

The border method would be good, but it will require applying the padding 2x
the offset to both the width and the height in the "slot" for the image
in the atlas, and then doing glCopyTexSubImage2d (assuming this exists in the
API) on the data to fill the additinal padding with the appropriate texture
data. This also implies that image widths and heights will no longer be
equivalent to the image's corresponding BSP region; the regions will be larger.

This likely affects a few tests made when the atlas is being constructed,
those will need to be accounted for.

What's also important is knowing what the proper width/height of the padding
should be in addition to knowing how that width/height will be sufficient.

Also an offset might need to be applied to every st coordinate in the shader
to prevent duplicates from showing - I'd have to think more about that to be
sure.

Lots to think about.

[RUNNING ON WINDOWS]
Remember that you edited the directory names of the assets so that every character is lowercase, up until the actual filename. You'll want to use the current bundle when
switching over to Windows and Linux.

[unnamed]
Chrome performance. Should do this immediately after finishing feature/effect_shaders.

[unnamed]
// Do view-space zDepth evaluation in `BSPRenderer::ProcessFace()` when the drawFace_t is created.

// Keep track of max/min view-space z-values for each face;
// ensure that closest point on face bounds (out of the 8 corners)
// relative to the view frustum is compared against max-z and farthest
// relative to the view frustum is compared against min-z.

// This is 8 matrix/vector multiplies (using the world->camera transform). Using SIMD and packing
// corner vectors into a 4D matrix you can do these ops in two.
// Get it working first, though.

[unnamed]
* Allocate a separate default shader for models which always blends the color mapped value in the texture with the vertex diffuse light/color.
