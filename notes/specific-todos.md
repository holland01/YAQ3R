[feature/effect_shaders]
* Go through each effect function entry in the lookup table found in effect_shader.cpp.
There should be a return false in the event that the evaluated token params aren't recognized (while assigning a default to keep things running).
Log these.

Also log any situations where a token key for the lookup table itself isn't recognized (this check is right before the table is used). 

[feature/effect_shaders]
* Review the section in the Q3 Shader Manual for the tcModScale effect; it should shed some light on issues happening with the 
renderer's current implementation.

[feature/effect_shaders]
Fix sky-related issues. Some of this is due to the way it's being texture mapped ( tex coords should be mapped according to a sphere with radius specified in a shader (skyparms) )

[feature/effect_shaders]
Fix lava rising effect.

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

