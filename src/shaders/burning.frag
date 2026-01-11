#version 330 core
in vec3 fragColor;
in float fragAlpha;
out vec4 FragColor;

// output billboard flame with alpha discard
void main()
{
    if(fragAlpha < 0.01) discard;
    FragColor = vec4(fragColor, fragAlpha);
}