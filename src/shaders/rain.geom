#version 330 core

layout (points) in;
layout (line_strip, max_vertices = 2) out;

in float vSpeed[];
in float vOffset[];

out vec4 gColor;

uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform float rainLength;  // raindrop length

void main()
{
    // get input point position
    vec4 position = gl_in[0].gl_Position;
    float speed = vSpeed[0];
    float offset = vOffset[0];

    // calculate current raindrop Y position (falling over time)
    float animTime = time + offset;
    float yPos = position.y - mod(animTime * speed, 200.0);

    // if raindrop falls below ground, reset to top
    if (yPos < -50.0) {
        yPos += 200.0;
    }

    // raindrop start point (top)
    vec4 startPos = vec4(position.x, yPos, position.z, 1.0);

    // raindrop end point (bottom, slightly shorter to form line)
    vec4 endPos = vec4(position.x, yPos - rainLength, position.z, 1.0);

    // raindrop effect with transparency gradient
    float alpha = smoothstep(-50.0, 50.0, yPos);

    // raindrop top - semi-transparent
    gColor = vec4(0.9, 0.95, 1.0, alpha * 0.5);
    gl_Position = projection * view * startPos;
    EmitVertex();

    // raindrop bottom - more transparent, forming trail effect
    gColor = vec4(0.9, 0.95, 1.0, alpha * 0.3);
    gl_Position = projection * view * endPos;
    EmitVertex();

    EndPrimitive();
}
