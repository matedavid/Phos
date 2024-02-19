#version 450

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTextureCoords;

#include "include/FrameUniforms.Vertex.glslh"

layout (location = 0) out vec2 vTextureCoords;
layout (location = 1) out vec3 vCameraPosition;

// layout (location = 2) out mat4 vLightSpaceMatrix;

//layout (push_constant) uniform LightingPassPushConstants {
//    mat4 lightSpaceMatrix;
//    mat4 model;
//} uLightingInfo;

void main() {
    gl_Position = vec4(aPosition, 1.0f);

    vTextureCoords = aTextureCoords;
    vCameraPosition = uCamera.position;
    // vLightSpaceMatrix = uLightingInfo.lightSpaceMatrix;
}
