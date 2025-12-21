#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in ivec4 aBoneIDs;
layout (location = 4) in vec4 aWeights;

const int MAX_BONES = 200;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 finalBonesMatrices[MAX_BONES];
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// TODO: Implement gouraud shading
uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;

out vec3 VertexColor;
out vec2 TexCoord;

void main()
{
    vec4 totalPosition = vec4(0.0f);
    vec3 totalNormal = vec3(0.0f);
    float totalWeight = 0.0f;
    
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(aBoneIDs[i] == -1) 
            continue;
        if(aBoneIDs[i] >= MAX_BONES) 
        {
            // Invalid bone ID, skip
            continue;
        }
        vec4 localPosition = finalBonesMatrices[aBoneIDs[i]] * vec4(aPos, 1.0f);
        totalPosition += localPosition * aWeights[i];
        vec3 localNormal = mat3(finalBonesMatrices[aBoneIDs[i]]) * aNormal;
        totalNormal += localNormal * aWeights[i];
        totalWeight += aWeights[i];
    }
    
    // If no bones affect this vertex or total weight is too small, use original position and normal
    if(totalWeight < 0.01)
    {
        totalPosition = vec4(aPos, 1.0f);
        totalNormal = aNormal;
    }

    // todo1
    // Transform to world space (after bone transformation)
    // Use totalPosition as vertex's input pos (aPos), to animation (change these)
    // totalNormal as vertex's input normal (aNormal)
    vec4 worldPos = model * totalPosition;
    vec3 FragPos = worldPos.xyz;

    // Get N, L, V, R
    vec3 N = normalize(mat3(transpose(inverse(model))) * totalNormal);
    vec3 L = normalize(lightPos - FragPos);
    vec3 R = reflect(-L, N);
    vec3 V = normalize(viewPos - FragPos);
    
    // Phong Reflection Model
    vec3 I_amb = lightAmbient * materialAmbient;
    float NdotL = max(dot(N, L), 0.0);
    vec3 I_diff = lightDiffuse * materialDiffuse * NdotL;
    float VdotR = max(dot(V, R), 0.0);
    float spec = pow(VdotR, materialShininess);
    vec3 I_spec = lightSpecular * materialSpecular * spec;
    
    VertexColor = I_amb + I_diff + I_spec;
    TexCoord = aTexCoord;
    gl_Position = projection * view * worldPos;
}
