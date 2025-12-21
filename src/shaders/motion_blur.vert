#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 model;

out VS_OUT {
    vec3 objectPos;  // 物体空间位置（用于motion blur重建）
    vec3 position;   // 世界空间位置
    vec3 normal;     // 世界空间法线
    vec2 texCoord;   // 纹理坐标
} vs_out;

void main()
{
    // 传递物体空间坐标
    vs_out.objectPos = aPos;

    // 将位置和法线转换到世界空间
    vec4 worldPos = model * vec4(aPos, 1.0);
    vs_out.position = worldPos.xyz;
    vs_out.normal = mat3(transpose(inverse(model))) * aNormal;
    vs_out.texCoord = aTexCoord;

    // 注意：不在这里进行view和projection变换，留给geometry shader处理
    gl_Position = worldPos;
}
