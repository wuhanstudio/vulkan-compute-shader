#version 430 core

layout (location = 0 ) in vec4 vColor;       // Interpolated color from vertex shader
layout (location = 0 ) out vec4 FragColor;   // Final fragment color

void main() {
    FragColor = vColor; // Set fragment color
}
