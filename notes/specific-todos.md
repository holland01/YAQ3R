
[priority]
* bugfix/any_wall_clip_weirdness 

verify that noshader is actually meant for drawing; if not, there will be faces which index into that shader and that will be drawn when they
shouldn't be. An alternative name you can use is just "defaultshader" or something like that, for rendering objects without effect shaders


[unnamed]
* ~Some textures are flipped upside down when they shouldn't be. There doesn't appear to be a tcMod setting which causes this to happen, and disabling
the 2D rotation effect by setting the matrix to its identity changes nothing. Swizzling all texture coordinates does produce what's desired for
those specific textures, however every other texture is messed up.~

~Likely want to assess how this will be interacting with the texture atlas, and then check Quake's code to see if there's any kind of coordinate swizzling
for specific textures; there could be a shaders for some of these images which have relevant content/surface flags, for example.~

[unnamed]
* Review the section in the Q3 Shader Manual for the tcModScale effect; it should shed some light on issues happening with the 
renderer's current implementation.

[unnamed]
* Allocate a separate default shader for models which always blends the color mapped value in the texture with the vertex diffuse light/color.


[unnamed]
* In `BSPRenderer::DrawEffectPass()`, there's this:

    ```
    if ( texIndex < 0 )
    {
        //FlagExit();
    //	MLOG_INFO( "texIndex: %i\n\n %s", texIndex, errorInfo.str().c_str() );
        return;
    }
    ```

Replace the return with a printf which confirms that texIndex < 0: you want
to make sure that no crash occurs when the atlas resolves to its default image.

[unnamed]
* Go through each effect function entry in the lookup table found in effect_shader.cpp.
There should be a return false in the event that the evaluated token params aren't recognized.
Log these.

Also log any situations where a token key for the lookup table itself isn't recognized (this check is right before the table is used). 
