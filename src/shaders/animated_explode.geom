#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 texCoord;
    vec3 normal;
} gs_in[];

out vec2 TexCoord;

uniform float time;
uniform float explodeStrength;  // 爆炸強度 (0.0 = 無效果, 1.0+ = 完全爆炸)

vec3 GetNormal()
{
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}

vec4 explode(vec4 position, vec3 normal)
{
    // 使用 explodeStrength 控制爆炸程度
    float magnitude = explodeStrength * 50.0;  // 調整爆炸距離
    vec3 direction = normal * magnitude;
    return position + vec4(direction, 0.0);
}

void main()
{
    vec3 normal = GetNormal();
    
    for(int i = 0; i < 3; i++){
        gl_Position = explode(gl_in[i].gl_Position, normal);
        TexCoord = gs_in[i].texCoord;
        EmitVertex();
    }
    
    EndPrimitive();
}
