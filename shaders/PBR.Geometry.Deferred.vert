#version 450

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTextureCoords;
layout (location = 3) in vec3 aTangent;

#include "include/FrameUniforms.Vertex.glslh"

layout (push_constant) uniform ModelInfoPushConstants {
    mat4 model;
    vec4 color;
};

layout (location = 0) out vec4 vPosition;
layout (location = 1) out vec2 vTextureCoords;
layout (location = 2) out mat3 vTBN;

void main() {
    gl_Position = uCamera.projection * uCamera.view * model * vec4(aPosition, 1.0f);

    vPosition = vec4(aPosition, 1.0f);
    vTextureCoords = aTextureCoords;

    vec3 T = normalize(vec3(model * vec4(aTangent, 0.0f)));
    vec3 N = normalize(vec3(model * vec4(aNormal, 0.0f)));
    vec3 B = cross(N, T);
    vTBN = mat3(T, B, N);
}
