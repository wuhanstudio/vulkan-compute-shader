#version 430 core

layout (location = 0) in vec2 in_position;

void main()
{
    gl_Position = vec4(in_position, 0.0, 1.0); 
    gl_PointSize = 10.0;                // Set point size
}
