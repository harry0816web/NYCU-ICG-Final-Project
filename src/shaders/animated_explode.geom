#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 texCoord;
    vec3 normal;
} gs_in[];

out vec2 TexCoord;

uniform float time;
uniform float explodeStrength;  // explosion intensity (0.0 = none, 1.0 = full)

vec3 GetNormal()
{
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}

vec4 explode(vec4 position, vec3 normal)
{
    // push triangle along its normal scaled by explodeStrength
    float magnitude = explodeStrength * 50.0;  // adjust explosion distance
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
