#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in ivec4 boneIds;
layout (location = 4) in vec4 weights;

const int MAX_BONES = 200;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

out VS_OUT {
    vec2 texCoord;
    vec3 normal;
} vs_out;

// vertex skinning for exploding geometry pass
void main()
{
    // bone animation transform
    vec4 totalPosition = vec4(0.0);
    vec3 totalNormal = vec3(0.0);
    
    for(int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        if(boneIds[i] == -1) continue;
        if(boneIds[i] >= MAX_BONES) {
            totalPosition = vec4(aPos, 1.0);
            totalNormal = aNormal;
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos, 1.0);
        totalPosition += localPosition * weights[i];
        vec3 localNormal = mat3(finalBonesMatrices[boneIds[i]]) * aNormal;
        totalNormal += localNormal * weights[i];
    }
    
    vs_out.texCoord = aTexCoord;
    vs_out.normal = mat3(transpose(inverse(model))) * totalNormal;
    gl_Position = projection * view * model * totalPosition;
}