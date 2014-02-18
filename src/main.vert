#version 420

in vec3 position;
in vec4 color;
in vec2 tex0;

uniform mat4 modelToCamera;
uniform mat4 cameraToClip;

smooth out vec4 frag_Color;
smooth out vec2 frag_Tex;

void main()
{
    gl_Position = cameraToClip * modelToCamera * vec4( position, 1.0 );

    frag_Color = color;
    frag_Tex = tex0;
}
