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

layout (location = 0) out vec3 vPosition;
layout (location = 1) out vec2 vTextureCoords;
layout (location = 2) out vec3 vNormal;
layout (location = 3) out mat3 vTBN;

void main() {
    gl_Position = uCamera.projection * uCamera.view * uModelInfo.model * vec4(aPosition, 1.0f);

    vPosition = vec3(uModelInfo.model * vec4(aPosition, 1.0f));
    vTextureCoords = aTextureCoords;
    vNormal = aNormal;

    vec3 T = normalize(vec3(uModelInfo.model * vec4(aTangent, 0.0f)));
    vec3 N = normalize(vec3(uModelInfo.model * vec4(aNormal, 0.0f)));
    vec3 B = cross(N, T);
    vTBN = mat3(T, B, N);
}
