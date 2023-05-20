#version 450

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTextureCoords;

layout (location = 0) out vec2 vTextureCoords;
layout (location = 1) out vec3 vCameraPosition;

layout (std140, set = 0, binding = 0) uniform CameraUniformBuffer {
    mat4 projection;
    mat4 view;
    vec3 position;
} uCamera;

void main() {
    gl_Position = vec4(aPosition, 1.0f);

    vTextureCoords = aTextureCoords;
    vCameraPosition = uCamera.position;
}