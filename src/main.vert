#version 420

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
//layout(location = 2) in vec2 tex0;

uniform mat4 modelToCamera;
uniform mat4 cameraToClip;

smooth out vec4 frag_Color;
//smooth out vec2 frag_Tex;

void main()
{
    gl_Position = cameraToClip * modelToCamera * vec4( position, 1.0 );

    frag_Color = color;
    //frag_Tex = tex0;
}
