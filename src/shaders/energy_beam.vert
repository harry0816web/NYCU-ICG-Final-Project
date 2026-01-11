#version 330 core

// feed per-beam attributes to geometry stage
layout (location = 0) in vec3 aDirection;   // energy direction (unit vector)
layout (location = 1) in float aSpeed;      // launch speed
layout (location = 2) in float aOffset;     // time offset
layout (location = 3) in vec3 aColor;       // rgb color

out float vSpeed;
out float vOffset;
out vec3 vColor;
out vec3 vDirection;

void main()
{
    // pass attributes through; geometry shader computes positions from center
    vDirection = aDirection;
    vSpeed = aSpeed;
    vOffset = aOffset;
    vColor = aColor;

    // placeholder position; actual line segment built in geometry shader
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
}
