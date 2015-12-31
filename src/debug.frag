#version 450

uniform vec4 fragColor;

out vec4 fragment;

const float gamma = 1.0 / 2.2;

void main()
{
    fragment = vec4( pow( fragColor.r, gamma ), pow( fragColor.g, gamma ), pow( fragColor.b, gamma ), fragColor.a );
}
