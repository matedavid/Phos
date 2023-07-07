#version 450

#include "include/LightInformation.glslh"

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec2 vTextureCoords;
layout (location = 2) in mat3 vTBN;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;
layout (location = 3) out vec4 outMetallicRoughnessAO;

layout (set = 2, binding = 0) uniform sampler2D uAlbedoMap;
layout (set = 2, binding = 1) uniform sampler2D uMetallicMap;
layout (set = 2, binding = 2) uniform sampler2D uRoughnessMap;
layout (set = 2, binding = 3) uniform sampler2D uAOMap;
layout (set = 2, binding = 4) uniform sampler2D uNormalMap;

void main() {
    outPosition = vPosition;

    vec3 normal = texture(uNormalMap, vTextureCoords).rgb;
    normal = normal * 2.0f - 1.0f; // convert from [0,1] to [-1,1]
    normal = normalize(vTBN * normal);
    outNormal.rgb = normal;

    outAlbedo = texture(uAlbedoMap, vTextureCoords);

    outMetallicRoughnessAO.r = texture(uMetallicMap, vTextureCoords).b;
    outMetallicRoughnessAO.g = texture(uRoughnessMap, vTextureCoords).g;
    outMetallicRoughnessAO.b = texture(uAOMap, vTextureCoords).r;
}