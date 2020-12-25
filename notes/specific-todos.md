priorities:

- bugfix/atlas_seam_filtering

- billboards

- implementing all rgbgen and alphagen methods

[feature/effect_shaders]
* Review the section in the Q3 Shader Manual for the tcModScale effect; it should shed some light on issues happening with the
renderer's current implementation.

[feature/effect_shaders]
Fix lava rising effect.

[bugfix/atlas_seam_filtering]
Downscale the image. Read [this](http://www.ogre3d.org/forums/viewtopic.php?f=4&t=61602) 

This is simpler to the bordered approach idea you had, except it's better since it chooses
a downscaling method as opposed to something which requires more memory.

You want to keep the area sizes that the images initially take up, but at the same time this will imply
that the offsets and scaling parameters passed to the shaders will need to be adjusted accordingly.

There's a function that's stubbed out in gl_atlas.h, line 367. Make sure that this is called 
before `fill_atlas_image()`, and that the corresponding shader parameters are correct (fetched via `image_info()`).

[RUNNING ON WINDOWS]
Remember that you edited the directory names of the assets so that every character is lowercase, up until the actual filename. You'll want to use the current bundle when
switching over to Windows and Linux.

[unnamed]
Billboards. Note that this will involve some work, but should be taken care of after bufix/atlas_seam_filtering.


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
