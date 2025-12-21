#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in float Alpha; // 接收 Geometry Shader 傳來的透明度

uniform sampler2D texture_diffuse;

void main() {
    vec4 texColor = texture(texture_diffuse, TexCoords);
    
    // 顏色不變，但透明度隨著 Alpha 改變
    FragColor = vec4(texColor.rgb, texColor.a * Alpha);
}