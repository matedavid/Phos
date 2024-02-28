#version 450

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTextureCoords;
layout (location = 3) in vec3 aTangent;

#include "include/FrameUniforms.Vertex.glslh"

layout (push_constant) uniform ModelInfoPushConstants {
    mat4 model;
    vec4 color;
} uModelInfo;

layout (location = 0) out vec3 vTextureCoords;

void main() {
    vTextureCoords = aPosition;

    mat4 view = mat4(mat3(uCamera.view));
    vec4 pos = uCamera.projection * view * uModelInfo.model * vec4(aPosition, 1.0f);
    gl_Position = pos.xyww;
}