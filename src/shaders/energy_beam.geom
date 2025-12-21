#version 330 core

layout (points) in;
layout (line_strip, max_vertices = 2) out;

in float vSpeed[];
in float vOffset[];
in vec3 vColor[];
in vec3 vDirection[];

out vec4 gColor;

uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform vec3 explosionCenter;  // 爆炸中心點
uniform float beamLength;      // 能量線長度

void main()
{
    vec3 direction = vDirection[0];
    float speed = vSpeed[0];
    float offset = vOffset[0];
    vec3 baseColor = vColor[0];  // 直接使用混合後的 RGB 顏色

    // 計算當前能量線的延伸距離（隨時間增長）
    float animTime = time + offset;
    float distance = animTime * speed;

    // 能量線的起始點（爆炸中心）
    vec4 startPos = vec4(explosionCenter, 1.0);

    // 能量線的結束點（沿方向延伸）
    vec4 endPos = vec4(explosionCenter + direction * distance, 1.0);

    // 計算能量線的尾部起點（形成線段，而非從中心射出的射線）
    float tailDistance = max(0.0, distance - beamLength);
    vec4 tailPos = vec4(explosionCenter + direction * tailDistance, 1.0);

    // 計算透明度（距離越遠越透明，模擬能量衰減）
    float alpha = smoothstep(150.0, 0.0, distance);

    // 能量線尾部 - 較暗
    gColor = vec4(baseColor * 0.5, alpha * 0.8);
    gl_Position = projection * view * tailPos;
    EmitVertex();

    // 能量線前端 - 較亮（頭部更亮，形成能量感）
    gColor = vec4(baseColor * 1.5, alpha);
    gl_Position = projection * view * endPos;
    EmitVertex();

    EndPrimitive();
}
