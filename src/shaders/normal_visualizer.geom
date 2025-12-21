#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 normal;
} gs_in[];

// 控制法線線段的長度
uniform float normalLength;

out vec3 fColor;

void GenerateLine(int index)
{
    // 起點：頂點位置 (黃色)
    fColor = vec3(1.0, 1.0, 0.0);
    gl_Position = gl_in[index].gl_Position;
    EmitVertex();
    
    // 終點：頂點位置 + 法線方向 (紅色)
    fColor = vec3(1.0, 0.0, 0.0);
    gl_Position = gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0) * 10;
    EmitVertex();
    
    EndPrimitive();
}

void main()
{
    // 為三角形的每個頂點生成一條法線
    GenerateLine(0);
    GenerateLine(1);
    GenerateLine(2);
}
