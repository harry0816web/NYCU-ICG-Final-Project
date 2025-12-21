#version 330 core

// TODO: Implement CubeMap shading
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
    // todo4-3: render color from 3d cube sampler
    FragColor = texture(skybox, TexCoords);
}
