#version 420

in vec3 position;
in vec4 color;
in vec2 tex0;
in vec2 lightmap;

uniform mat4 modelToCamera;
uniform mat4 cameraToClip;

smooth out vec4 frag_Color;
smooth out vec2 frag_Tex;
smooth out vec2 frag_Lightmap;

uniform bool doGammaCorrect;

const float gammaExp = 1.0 / 2.2;

void main()
{
    gl_Position = cameraToClip * modelToCamera * vec4( position, 1.0 );

	if ( doGammaCorrect )
		frag_Color = pow( color, vec4( gammaExp ) );
    else
		frag_Color = color;

	frag_Tex = tex0;
	frag_Lightmap = lightmap;
}
