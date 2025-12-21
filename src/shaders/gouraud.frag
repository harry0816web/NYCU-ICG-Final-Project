#version 330 core

// TODO:
// Implement Gouraud shading
out vec4 FragColor;

in vec3 VertexColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
    vec3 color = texture(ourTexture, TexCoord).rgb;
    FragColor = vec4(VertexColor * color, 1.0);
}
