#version 450

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTextureCoords;
layout (location = 3) in vec3 aTangent;

#include "include/FrameUniforms.Vertex.glslh"

layout (push_constant) uniform ShadowMapPushConstants {
    mat4 lightSpaceMatrix;
    mat4 model;
} uShadowMapInfo;

void main() {
    gl_Position = uShadowMapInfo.lightSpaceMatrix * uShadowMapInfo.model * vec4(aPosition, 1.0f);
}