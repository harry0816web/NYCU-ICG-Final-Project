#version 330 core
// pass world-space data to geometry shader for trail reconstruction
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 model;

out VS_OUT {
    vec3 objectPos;  // object space position (for motion blur reconstruction)
    vec3 position;   // world space position
    vec3 normal;     // world space normal
    vec2 texCoord;   // texture coordinates
} vs_out;

void main()
{
    // pass object space coordinates
    vs_out.objectPos = aPos;

    // transform position and normal to world space
    vec4 worldPos = model * vec4(aPos, 1.0);
    vs_out.position = worldPos.xyz;
    vs_out.normal = mat3(transpose(inverse(model))) * aNormal;
    vs_out.texCoord = aTexCoord;

    // note: do not perform view and projection transformation here, leave it to geometry shader
    gl_Position = worldPos;
}
