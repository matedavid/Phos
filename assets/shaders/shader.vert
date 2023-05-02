#version 450

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTextureCoords;
layout (location = 3) in vec3 aTangent;


layout (set = 0, binding = 0) uniform CameraUniformBuffer {
    mat4 projection;
    mat4 view;
};

void main() {
    gl_Position = projection * view * vec4(aPosition, 1.0);
}