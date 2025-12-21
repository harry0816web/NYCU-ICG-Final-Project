// Example of how the animated vertex shader works vs original

// ORIGINAL vertex shader (default.vert):
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    // ... rest of shader
}

// ANIMATED vertex shader (animated_default.vert):
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in ivec4 aBoneIDs;      // NEW: Bone IDs
layout (location = 4) in vec4 aWeights;       // NEW: Bone weights

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 finalBonesMatrices[MAX_BONES];   // NEW: Bone transformation matrices

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // NEW: Apply bone transformations
    vec4 totalPosition = vec4(0.0f);
    for(int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        if(aBoneIDs[i] == -1) continue;
        if(aBoneIDs[i] >= MAX_BONES) {
            totalPosition = vec4(aPos, 1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[aBoneIDs[i]] * vec4(aPos, 1.0f);
        totalPosition += localPosition * aWeights[i];
    }
    
    // Fallback to original position if no bones
    if(length(totalPosition) == 0.0) {
        totalPosition = vec4(aPos, 1.0f);
    }

    // Use animated position instead of original
    gl_Position = projection * view * model * totalPosition;
    // ... rest of shader remains the same
}
