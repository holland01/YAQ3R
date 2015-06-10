#version 420

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec4 color;
layout( location = 2 ) in vec2 tex0;
layout( location = 3 ) in vec2 lightmap;

uniform mat4 modelViewProjection;

smooth out vec4 frag_Color;
smooth out vec2 frag_Tex;
smooth out vec2 frag_Lightmap;

void main(void)
{
	gl_Position = modelViewProjection * vec4( position, 1.0 );

	frag_Color = color;
    frag_Tex = tex0;
    frag_Lightmap = lightmap;
}