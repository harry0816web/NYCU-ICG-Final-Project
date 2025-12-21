#version 330 core

layout (location = 0) in vec3 aPos;      // 雨滴起始位置
layout (location = 1) in float aSpeed;   // 雨滴下落速度
layout (location = 2) in float aOffset;  // 时间偏移量

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
