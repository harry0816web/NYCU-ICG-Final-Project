#version 330 core

layout (location = 0) in vec3 aPos;      // raindrop initial position
layout (location = 1) in float aSpeed;   // raindrop fall speed
layout (location = 2) in float aOffset;  // time offset

out float vSpeed;
out float vOffset;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    vSpeed = aSpeed;
    vOffset = aOffset;
}
