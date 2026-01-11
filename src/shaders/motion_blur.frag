#version 330 core
// sample diffuse texture and modulate alpha from motion trails
out vec4 FragColor;

in vec2 TexCoords;
in float Alpha; // receive transparency from geometry shader

uniform sampler2D texture_diffuse;

void main() {
    vec4 texColor = texture(texture_diffuse, TexCoords);
    
    // color unchanged, but transparency scales with geometry-provided alpha
    FragColor = vec4(texColor.rgb, texColor.a * Alpha);
}