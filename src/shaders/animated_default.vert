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

out vec2 TexCoord;

void main()
{
    vec4 totalPosition = vec4(0.0f);
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
        totalWeight += aWeights[i];
    }
    
    // If no bones affect this vertex or total weight is too small, use original position
    if(totalWeight < 0.01)
    {
        totalPosition = vec4(aPos, 1.0f);
    }

    // Use totalPosition as vertex's input pos (aPos)
    // totalNormal as vertex's input normal (aNormal)
    gl_Position = projection * view * model * totalPosition;
    TexCoord = aTexCoord;
}
