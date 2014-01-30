#version 420

layout(location = 0) in vec3 position;

uniform mat4 modelview;
uniform mat4 projection;

out vec4 frag_Color;

void main(void)
{
    gl_Position = projection * modelview * vec4( position, 1.0 );

    frag_Color = vec4( 0.0, 1.0, 1.0, 1.0 );
}
