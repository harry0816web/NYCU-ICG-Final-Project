#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 texCoord;
} gs_in[];

out vec2 TexCoord;

uniform float time;

vec3 GetNormal()
{
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}

vec4 explode(vec4 position, vec3 normal)
{
    vec3 direction = normal * (sin(time)+1) * 100;
    return position + vec4(direction, 0.0);
}

void main()
{
    vec3 normal = GetNormal();
    
    for(int i = 0; i<3; i++){
        gl_Position = explode(gl_in[i].gl_Position, normal);
        TexCoord = gs_in[0].texCoord;
        EmitVertex();
    }
    
    EndPrimitive();
}
