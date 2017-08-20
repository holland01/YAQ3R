[feature/effect_shaders]
* Review the section in the Q3 Shader Manual for the tcModScale effect; it should shed some light on issues happening with the 
renderer's current implementation.

[feature/effect_shaders]
Review the indices/triangle ordering that's happening. It's all kind of shitty. Source of problem could be due to index ordering
or something like face culling being turned on AND improper front faces since the winding order might be CCW (I can't remember).

Don't forget to set the defer flag in the sky shader stage program after it's been inserted in effect_shader.cpp

[feature/effect_shaders]
Fix lava rising effect.

[feature/effect_shaders]
Make sure to test this on Linux/Windows. There's some odd artificats, notably with the cross texture and the scrolling which occurs 
across the bridge. Rainbow/garbage decals are appearing. 

This might be due to any of the following:

- Corrupted bundles
- driver error (unlikely)
- missing rgbgen or alphagen-related GLSL code
- bad param offset/index.

Maybe see if passing 1.0f in timeScalarSeconds eliminates this, to ensure nothing is getting sampled out of bounds. (the GLSL generated code should take this into account, but some resulting
st coords may be too small to be of any actual use)

[RUNNING ON LINUX/WINDOWS]
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



