#version 330 core

// TODO: Implement metallic shading

// Change: Metallic fragment shader implementation
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform samplerCube skybox;
uniform sampler2D ourTexture;

// Metallic shading parameters
uniform float bias;      // 0.2
uniform float alpha;     // 0.4
uniform float lightIntensity;  // I_l = 1.0

void main()
{
    // todo5-2: get view direction from camera to fragment
    vec3 N = normalize(Normal);
    vec3 I = normalize(FragPos - viewPos); 
    
    // calculate reflection vector and sample cubemap color
    vec3 R = reflect(I, N);
    vec3 C_reflect = texture(skybox, R).rgb;
    // without model color
    vec3 C_final = C_reflect;
    
    FragColor = vec4(C_final, 1.0);
}	
