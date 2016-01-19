#version 100

uniform vec4 fragColor;

const float gamma = 1.0 / 2.2;

void main()
{
    gl_FragColor = vec4( pow( fragColor.r, gamma ), pow( fragColor.g, gamma ), pow( fragColor.b, gamma ), fragColor.a );
}
