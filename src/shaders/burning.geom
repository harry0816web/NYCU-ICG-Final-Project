#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 16) out;

in VS_OUT {
    vec3 position;
} gs_in[];

out vec3 fragColor;
out float fragAlpha;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform float carCollisionTime; // Time when collision happened

// Random functions
float random(float seed) {
    return fract(sin(seed * 78.233 + time * 0.1) * 43758.5453);
}

void main()
{
    float offsetTime = time - carCollisionTime;
    
    // If not collided yet or just collided, maybe delay slightly or start immediately
    if (offsetTime < 0.0) return;

    // Use triangle center
    vec3 center = (gs_in[0].position + gs_in[1].position + gs_in[2].position) / 3.0;
    vec4 worldPos = model * vec4(center, 1.0);

    // Filter: only emit from top part or random parts to simulate fire locations?
    // Let's emit from everywhere for now, but maximize on top?
    // Or just simple uniform emission.

    // Number of particles per triangle
    int count = 1; 
    
    // Randomly skip some triangles to control density if needed
    if(random(center.x + center.z) > 0.3) count = 0; // 70% chance to skip

    for(int i=0; i<count; i++) {
        float seed = float(i) + center.x * 12.3 + center.z * 45.6;
        
        // Particle lifecycle
        float life = 2.0; // seconds
        // Each particle has a random start offset in the cycle
        float startOffset = random(seed) * life;
        float t = mod(offsetTime + startOffset, life);
        float progress = t / life;

        // Position: rise up
        vec3 pos = worldPos.xyz;
        // Jitter starting position
        pos.x += (random(seed+1.0)-0.5) * 5.0; 
        pos.z += (random(seed+2.0)-0.5) * 5.0;
        pos.y += t * 40.0; // Rise speed, up to 80 units high
        
        // Jitter motion (wind/turbulence)
        pos.x += sin(t * 3.0 + seed) * 5.0;

        // Size: shrinks
        float size = 3.0 * (1.0 - progress * 0.5);

        // Alpha: fade out
        float alpha = 1.0 - progress;
        alpha *= 0.8; // Max alpha
        
        // Color: Fire colors (Yellow -> Red -> Smoke)
        vec3 color;
        if(progress < 0.2) color = vec3(1.0, 0.9, 0.2); // Yellow
        else if(progress < 0.5) color = vec3(1.0, 0.4, 0.0); // Orange
        else if(progress < 0.8) color = vec3(0.8, 0.1, 0.0); // Red
        else color = vec3(0.4, 0.4, 0.4); // Grey smoke

        fragColor = color;
        fragAlpha = alpha;

        // Billboard
        vec3 right = vec3(view[0][0], view[1][0], view[2][0]);
        vec3 up = vec3(view[0][1], view[1][1], view[2][1]);

        gl_Position = projection * view * vec4(pos + (-right - up) * size, 1.0);
        EmitVertex();
        gl_Position = projection * view * vec4(pos + (right - up) * size, 1.0);
        EmitVertex();
        gl_Position = projection * view * vec4(pos + (-right + up) * size, 1.0);
        EmitVertex();
        gl_Position = projection * view * vec4(pos + (right + up) * size, 1.0);
        EmitVertex();

        EndPrimitive();
    }
}