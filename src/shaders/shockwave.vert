#version 330 core

layout (location = 0) in float aStartRadius;
layout (location = 1) in float aSpeed;
layout (location = 2) in float aOffset;
layout (location = 3) in float aThickness;
layout (location = 4) in vec3 aColor;

out float vStartRadius;
out float vSpeed;
out float vOffset;
out float vThickness;
out vec3 vColor;

void main()
{
    vStartRadius = aStartRadius;
    vSpeed = aSpeed;
    vOffset = aOffset;
    vThickness = aThickness;
    vColor = aColor;

    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
}
