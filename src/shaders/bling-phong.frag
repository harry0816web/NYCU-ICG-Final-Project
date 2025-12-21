#version 330 core

// TODO: Implement bling-phong shading
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;
uniform sampler2D ourTexture;

void main()
{
    // todo3-2: get tex color
    vec3 color = texture(ourTexture, TexCoord).rgb;
    
    // get N, L, V
    vec3 N = normalize(Normal);
    vec3 L = normalize(lightPos - FragPos);
    vec3 V = normalize(viewPos - FragPos);

    // todo3-3: phong reflection model
    vec3 I_amb = lightAmbient * materialAmbient;
    float NdotL = max(dot(N, L), 0.0);
    vec3 I_diff = lightDiffuse * materialDiffuse * NdotL;
    
    // todo3-3: bling-phong, use halfway Vector
    vec3 H = normalize(L + V);
    float NdotH = max(dot(N, H), 0.0);
    float spec = pow(NdotH, materialShininess);
    vec3 I_spec = lightSpecular * materialSpecular * spec;
    
    vec3 result = (I_amb + I_diff + I_spec) * color;
    FragColor = vec4(result, 1.0);
}
