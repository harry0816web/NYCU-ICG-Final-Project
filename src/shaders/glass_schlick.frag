#version 330 core

// TODO: Implement glass shading with schlick method
// Change: Glass fragment shader implementation with Fresnel (Schlick Approximation)
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 viewPos;
uniform samplerCube skybox;

// Refractive indices
uniform float eta;  // n1/n2 = 1.0/1.52

void main()
{          
    // todo6-2: use Schlick to approximate Fresnel relfectance
    vec3 N = normalize(Normal);
    vec3 I = normalize(FragPos - viewPos); 
    
    // Fresnel (Schlick Approximation)
    // R0 = ((n1 - n2) / (n1 + n2))^2 with AIR_coeff = 1, GLASS_coeff = 1.52
    float n1 = 1.0;  // Air
    float n2 = 1.52; // Glass
    float R0 = pow((n1 - n2) / (n1 + n2), 2.0);
    
    // R_theta = R0 + (1 - R0) * (1 + I·N)^5
    float I_dot_N = dot(I, N);
    float R_theta = R0 + (1.0 - R0) * pow(1.0 + I_dot_N, 5.0);
    
    // get C_reflect
    vec3 R = reflect(I, N);
    vec3 C_reflect = texture(skybox, R).rgb;
    
    // get C_refraction
    vec3 T = refract(I, N, eta);
    vec3 C_refract = texture(skybox, T).rgb;
    
    // mix the color to R_theta * C_reflect + (1 - R_theta) * C_refract
    vec3 C_final = R_theta * C_reflect + (1.0 - R_theta) * C_refract;
    
    FragColor = vec4(C_final, 1.0);
}  
