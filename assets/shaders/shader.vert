#version 450

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTextureCoords;
layout (location = 3) in vec3 aTangent;

layout (std140, set = 0, binding = 0) uniform CameraUniformBuffer {
    mat4 projection;
    mat4 view;
    vec3 position;
};

layout (location = 0) out vec3 vPosition;
layout (location = 1) out vec3 vNormal;
layout (location = 2) out vec3 vCameraPosition;

void main() {
    gl_Position = projection * view * vec4(aPosition, 1.0);

    vPosition = aPosition;
    vNormal = aNormal;
    vCameraPosition = position;
}