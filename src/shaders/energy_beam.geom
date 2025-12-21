#version 330 core

layout (points) in;
layout (line_strip, max_vertices = 2) out;

in float vSpeed[];
in float vOffset[];
in vec3 vColor[];
in vec3 vDirection[];

out vec4 gColor;

uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform vec3 explosionCenter;  // explosion center point
uniform float beamLength;      // energy beam length

void main()
{
    vec3 direction = vDirection[0];
    float speed = vSpeed[0];
    float offset = vOffset[0];
    vec3 baseColor = vColor[0];  // directly use mixed RGB color

    // calculate current energy beam extension distance (grows over time)
    float animTime = time + offset;
    float distance = animTime * speed;

    // energy beam start point (explosion center)
    vec4 startPos = vec4(explosionCenter, 1.0);

    // energy beam end point (extended along direction)
    vec4 endPos = vec4(explosionCenter + direction * distance, 1.0);

    // calculate energy beam tail start point (forms line segment, not ray from center)
    float tailDistance = max(0.0, distance - beamLength);
    vec4 tailPos = vec4(explosionCenter + direction * tailDistance, 1.0);

    // calculate transparency (farther distance more transparent, simulate energy decay)
    float alpha = smoothstep(150.0, 0.0, distance);

    // energy beam tail - darker
    gColor = vec4(baseColor * 0.5, alpha * 0.8);
    gl_Position = projection * view * tailPos;
    EmitVertex();

    // energy beam front - brighter (head brighter, forms energy feeling)
    gColor = vec4(baseColor * 1.5, alpha);
    gl_Position = projection * view * endPos;
    EmitVertex();

    EndPrimitive();
}
