#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

out VS_OUT {
    vec3 normal;
} vs_out;

void main()
{
    // transform normal from model space to clip space
    mat3 normalMatrix = mat3(transpose(inverse(view * model)));
    vs_out.normal = normalize(vec3(projection * vec4(normalMatrix * aNormal, 0.0)));
    
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
