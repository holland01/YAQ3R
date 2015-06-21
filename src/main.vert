#version 450

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec4 color;
layout( location = 2 ) in vec2 tex0;
layout( location = 3 ) in vec2 lightmap;
//layout( location = 4 ) in vec3 normal;

layout( std140 ) uniform Transforms
{
	mat4 viewToClip;
	mat4 modelToView;
};

smooth out vec4 frag_Color;
smooth out vec2 frag_Tex;
smooth out vec2 frag_Lightmap;
//smooth out vec3 frag_Normal;

void main()
{
    gl_Position = viewToClip * modelToView * vec4( position, 1.0 );
	
    frag_Color = color;
    frag_Tex = tex0;
	frag_Lightmap = lightmap;
	//frag_Normal = normalize( normal );
}
