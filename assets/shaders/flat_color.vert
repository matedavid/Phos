#version 450

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTextureCoords;
layout (location = 3) in vec3 aTangent;

layout (std140, set = 0, binding = 0) uniform CameraUniformBuffer {
    mat4 projection;
    mat4 view;
    vec3 position;
} uCamera;

layout (push_constant) uniform ModelInfoPushConstants {
    mat4 model;
    vec4 color;
};

layout (location = 0) out vec4 vPosition;
layout (location = 1) out vec3 vNormal;
layout (location = 2) out vec4 vColor;

void main() {
    gl_Position = uCamera.projection * uCamera.view * model * vec4(aPosition, 1.0f);

    vPosition = gl_Position;
    vNormal = aNormal;
    vColor = color;
}
