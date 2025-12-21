#version 330 core
layout (location = 0) in vec3 aPos;

out VS_OUT {
    vec3 position;
} vs_out;

void main()
{
    vs_out.position = aPos;
}