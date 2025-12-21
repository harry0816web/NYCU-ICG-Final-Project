#version 330 core
layout (location = 0) in vec3 aPos;

// TODO: Implement CubeMap shading
out vec3 TexCoords;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aPos;
    // todo4-2: skybox
    // mat3() to reove translation for view, only takes rotation
    mat4 view_removeTrans = mat4(mat3(view));
    vec4 pos = projection * view_removeTrans * vec4(aPos, 1.0);
    // Set z to w so that depth is always 1.0 -> make sure it's behind objects
    gl_Position = pos.xyww;
}
