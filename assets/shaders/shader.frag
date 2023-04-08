#version 450

layout (location = 0) out vec4 ResultColor;

layout (binding = 0) uniform ColorUniformBuffer {
    vec4 Color;
};

void main() {
    ResultColor = Color;
}