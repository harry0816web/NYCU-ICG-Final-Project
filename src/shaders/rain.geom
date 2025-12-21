#version 330 core

layout (points) in;
layout (line_strip, max_vertices = 2) out;

in float vSpeed[];
in float vOffset[];

out vec4 gColor;

uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform float rainLength;  // 雨滴长度

void main()
{
    // 获取输入点的位置
    vec4 position = gl_in[0].gl_Position;
    float speed = vSpeed[0];
    float offset = vOffset[0];

    // 计算当前雨滴的 Y 位置 (随时间下落)
    float animTime = time + offset;
    float yPos = position.y - mod(animTime * speed, 200.0);

    // 如果雨滴掉到地面以下,重置到顶部
    if (yPos < -50.0) {
        yPos += 200.0;
    }

    // 雨滴的起始点 (顶部)
    vec4 startPos = vec4(position.x, yPos, position.z, 1.0);

    // 雨滴的结束点 (底部,稍微短一些形成线条)
    vec4 endPos = vec4(position.x, yPos - rainLength, position.z, 1.0);

    // 带透明度渐变的雨滴效果
    float alpha = smoothstep(-50.0, 50.0, yPos);

    // 雨滴顶部 - 半透明
    gColor = vec4(0.9, 0.95, 1.0, alpha * 0.5);
    gl_Position = projection * view * startPos;
    EmitVertex();

    // 雨滴底部 - 更透明,形成拖尾效果
    gColor = vec4(0.9, 0.95, 1.0, alpha * 0.3);
    gl_Position = projection * view * endPos;
    EmitVertex();

    EndPrimitive();
}
