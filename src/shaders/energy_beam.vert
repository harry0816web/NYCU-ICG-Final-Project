#version 330 core

layout (location = 0) in vec3 aDirection;   // 能量線方向（單位向量）
layout (location = 1) in float aSpeed;      // 射出速度
layout (location = 2) in float aOffset;     // 時間偏移量
layout (location = 3) in vec3 aColor;       // RGB 顏色

out float vSpeed;
out float vOffset;
out vec3 vColor;
out vec3 vDirection;

void main()
{
    // 將方向傳遞給 geometry shader（在 geometry shader 中會使用中心點計算實際位置）
    vDirection = aDirection;
    vSpeed = aSpeed;
    vOffset = aOffset;
    vColor = aColor;

    // 暫時設置一個佔位位置，實際位置會在 geometry shader 中計算
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
}
