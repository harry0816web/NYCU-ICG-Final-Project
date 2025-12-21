#version 330 core

layout (points) in;
layout (line_strip, max_vertices = 65) out;

in float vStartRadius[];
in float vSpeed[];
in float vOffset[];
in float vThickness[];
in vec3 vColor[];

out vec4 gColor;

uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform vec3 shockwaveCenter;

void main()
{
    float startRadius = vStartRadius[0];
    float speed = vSpeed[0];
    float offset = vOffset[0];
    float thickness = vThickness[0];
    vec3 baseColor = vColor[0];

    // 計算當前衝擊波環的半徑
    float animTime = time + offset;
    float radius = startRadius + animTime * speed;

    // 透明度隨距離衰減
    float alpha = smoothstep(80.0, 0.0, radius);

    // 如果環已經消失，不渲染
    if (alpha <= 0.01) {
        return;
    }

    // 生成圓環（在地面 XZ 平面上）
    const float PI = 3.14159265359;
    int segments = 64;

    for (int i = 0; i <= segments; i++) {
        float angle = (float(i) / float(segments)) * 2.0 * PI;
        float x = cos(angle) * radius;
        float z = sin(angle) * radius;

        // 環的位置（shockwaveCenter 已經包含高度）
        vec3 position = shockwaveCenter + vec3(x, 0.0, z);

        // 顏色從內到外變化（中心更亮）
        float brightness = 1.0 + (1.0 - smoothstep(0.0, 50.0, radius)) * 0.5;
        gColor = vec4(baseColor * brightness, alpha);

        gl_Position = projection * view * vec4(position, 1.0);
        EmitVertex();
    }

    EndPrimitive();
}
