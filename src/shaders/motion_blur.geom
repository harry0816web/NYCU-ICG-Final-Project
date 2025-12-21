#version 330 core
layout (triangles) in;
// note: if you want to generate 5 layers of motion trails, max_vertices should be at least 3 * 5 = 15
layout (triangle_strip, max_vertices = 15) out;

in VS_OUT {
    vec3 objectPos;  // object space position
    vec3 position;   // world space position
    vec3 normal;     // world space normal
    vec2 texCoord;   // texture coordinates
} gs_in[];

out vec2 TexCoords;
out float Alpha; // used to control motion trail transparency

uniform float time;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;  // current model matrix

const float PI = 3.14159265359;

// reconstruct model matrix based on time (reproduce object movement)
mat4 getModelMatrixAtTime(float t) {
    // reproduce movement logic from UpdateCartMovement in cinematic_director.cpp
    // before 6s: stationary at (0, 0, 150)
    // 6-10s: move from (0, 0, 150) to (0, 0, 70)
    // after 10s: stationary at (0, 0, 70)
    
    float cartMoveStartTime = 6.0;
    float cartMoveDuration = 4.0; // 4 seconds to complete movement
    vec3 startPos = vec3(0.0, 0.0, 150.0);
    vec3 endPos = vec3(0.0, 0.0, 70.0);
    
    vec3 currentPos;
    if (t < cartMoveStartTime) {
        // before 6s: keep initial position
        currentPos = startPos;
    } else if (t > cartMoveStartTime + cartMoveDuration) {
        // after 10s: keep end position
        currentPos = endPos;
    } else {
        // 6-10s: linear interpolation movement
        float progress = (t - cartMoveStartTime) / cartMoveDuration;
        // use smooth function (consistent with C++ code)
        float smoothProgress = progress * progress * (3.0 - 2.0 * progress);
        currentPos = mix(startPos, endPos, smoothProgress);
    }
    
    mat4 m = mat4(1.0);
    
    // 1. scale (10x)
    m = mat4(
        10.0, 0.0, 0.0, 0.0,
        0.0, 10.0, 0.0, 0.0,
        0.0, 0.0, 10.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    
    // 2. rotate (180 degrees around Y axis)
    float angleRad = radians(180.0);
    float c = cos(angleRad);
    float s = sin(angleRad);
    mat4 rotate = mat4(
        c, 0.0, s, 0.0,
        0.0, 1.0, 0.0, 0.0,
        -s, 0.0, c, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    m = rotate * m;
    
    // 3. translate (to current position)
    mat4 translate = mat4(1.0);
    translate[3] = vec4(currentPos, 1.0);
    m = translate * m;
    
    return m;
}

void main() {
    // set number of motion trail layers
    int layers = 5;
    float timeLag = 0.15; // time delay for each motion trail layer

    // loop through each motion trail layer
    for(int i = 0; i < layers; i++) {

        // later layers (larger i) have earlier time (time - lag), lower Alpha
        float current_time = time - (float(i) * timeLag);

        float current_alpha = (2.0 - (float(i) / float(layers - 1)))/2;

        // draw three vertices of triangle
        for(int j = 0; j < 3; j++) {
            // 1. get model transformation matrix at that time point
            mat4 pastModel = getModelMatrixAtTime(current_time);

            // 2. apply past model matrix using object space coordinates
            vec4 worldPos = pastModel * vec4(gs_in[j].objectPos, 1.0);

            // 3. transform world space position to clip space
            gl_Position = projection * view * worldPos;

            // 4. pass data
            TexCoords = gs_in[j].texCoord;
            Alpha = current_alpha;

            EmitVertex();
        }
        EndPrimitive(); // each motion trail layer is an independent triangle strip
    }
}