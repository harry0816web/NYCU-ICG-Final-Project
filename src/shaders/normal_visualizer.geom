#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 normal;
} gs_in[];

// control normal line segment length
uniform float normalLength;

out vec3 fColor;

void GenerateLine(int index)
{
    // start point: vertex position (yellow)
    fColor = vec3(1.0, 1.0, 0.0);
    gl_Position = gl_in[index].gl_Position;
    EmitVertex();
    
    // end point: vertex position + normal direction (red)
    fColor = vec3(1.0, 0.0, 0.0);
    gl_Position = gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0) * 10;
    EmitVertex();
    
    EndPrimitive();
}

void main()
{
    // generate one normal line for each vertex of the triangle
    GenerateLine(0);
    GenerateLine(1);
    GenerateLine(2);
}
