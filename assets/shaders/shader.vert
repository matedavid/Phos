#version 450

layout (location = 0) in vec3 inPosition;

layout(binding = 0) uniform Camera {
    mat4 Projection;
    mat4 View;
    mat4 Position;
} camera;

void main() {
    gl_Position = vec4(inPosition, 1.0);
}