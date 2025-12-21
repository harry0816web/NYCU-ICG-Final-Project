#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in float Alpha; // receive transparency from geometry shader

uniform sampler2D texture_diffuse;

void main() {
    vec4 texColor = texture(texture_diffuse, TexCoords);
    
    // color unchanged, but transparency changes with Alpha
    FragColor = vec4(texColor.rgb, texColor.a * Alpha);
}