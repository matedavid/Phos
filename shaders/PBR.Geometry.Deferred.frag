#version 450

#include "include/LightInformation.glslh"

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vTextureCoords;
layout (location = 2) in vec3 vNormal;
layout (location = 3) in mat3 vTBN;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;
layout (location = 3) out vec4 outMetallicRoughnessAO;
layout (location = 4) out vec4 outEmission;

layout (set = 2, binding = 0) uniform sampler2D uAlbedoMap;
layout (set = 2, binding = 1) uniform sampler2D uMetallicMap;
layout (set = 2, binding = 2) uniform sampler2D uRoughnessMap;
layout (set = 2, binding = 3) uniform sampler2D uAOMap;
layout (set = 2, binding = 4) uniform sampler2D uNormalMap;

layout (set = 2, binding = 5) uniform PBRMaterialInfo {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;

    float emissionIntensity;
    vec3 emissionColor;
} uMaterialInfo;

void main() {
    outPosition = vec4(vPosition, 1.0f);

    vec3 normal = texture(uNormalMap, vTextureCoords).rgb;
    if (normal == vec3(1.0f)) {
        outNormal.rgb = vNormal;
    } else {
        normal = normal * 2.0f - 1.0f; // convert from [0,1] to [-1,1]
        normal = normalize(vTBN * normal);

        outNormal.rgb = normal;
    }

    outAlbedo = texture(uAlbedoMap, vTextureCoords) * vec4(uMaterialInfo.albedo, 1.0f);

    outMetallicRoughnessAO.r = texture(uMetallicMap, vTextureCoords).r * uMaterialInfo.metallic;
    outMetallicRoughnessAO.g = texture(uRoughnessMap, vTextureCoords).r * uMaterialInfo.roughness;
    outMetallicRoughnessAO.b = texture(uAOMap, vTextureCoords).r * uMaterialInfo.ao;
    outEmission = vec4(uMaterialInfo.emissionColor * uMaterialInfo.emissionIntensity, 1.0f);
}