#version 330 core
out vec4 FragColor;

in vec2 TexCoord; 

// sample diffuse map for exploded mesh
uniform sampler2D ourTexture;

void main()
{
    FragColor = texture(ourTexture, TexCoord);
} 