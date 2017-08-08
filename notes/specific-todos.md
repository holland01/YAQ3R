
[priority]
1) bugfix/any_wall_clip_weirdness

[unnamed]
2) In `BSPRenderer::DrawEffectPass()`, there's this:

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
3) Go through each effect function entry in the lookup table found in effect_shader.cpp.
There should be a return false in the event that the evaluated token params aren't recognized.
Log these.

Also log any situations where a token key for the lookup table itself isn't recognized (this check is right before the table is used). 
