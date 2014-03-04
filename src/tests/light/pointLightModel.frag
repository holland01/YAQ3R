#version 420

uniform vec4 objectColor;

out vec4 fragment;

void main(void)
{
    fragment = objectColor;
}
