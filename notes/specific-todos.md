1) In `BSPRenderer::DrawEffectPass()`, there's this:

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
