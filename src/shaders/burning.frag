#version 330 core
in vec3 fragColor;
in float fragAlpha;
out vec4 FragColor;

void main()
{
    if(fragAlpha < 0.01) discard;
    FragColor = vec4(fragColor, fragAlpha);
}