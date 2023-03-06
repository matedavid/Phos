#version 450

layout (location = 0) out vec4 ResultColor;

layout (location = 0) in vec3 FragColor;

void main() {
    ResultColor = vec4(FragColor, 1.0);
}