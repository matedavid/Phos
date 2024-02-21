#version 450

layout (location = 0) in vec2 vTextureCoords;
layout (location = 1) in vec3 vCameraPosition;
// layout (location = 2) in mat4 vLightSpaceMatrix;

layout (location = 0) out vec4 outColor;

#include "include/LightInformation.glslh"

layout (set = 1, binding = 0) uniform sampler2D uPositionMap;
layout (set = 1, binding = 1) uniform sampler2D uNormalMap;
layout (set = 1, binding = 2) uniform sampler2D uAlbedoMap;
layout (set = 1, binding = 3) uniform sampler2D uMetallicRoughnessAOMap;
layout (set = 1, binding = 4) uniform sampler2D uEmissionMap;

layout (set = 1, binding = 5) uniform ShadowMappingUniformBuffer {
    mat4 directionalLightSpaceMatrices[MAX_DIRECTIONAL_LIGHTS];
    int numberDirectionalShadowMaps;
} uShadowMappingInfo;
layout (set = 1, binding = 6) uniform sampler2D uDirectionalShadowMaps;

// Constants
const float PI = 3.14159265359;

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    // Square roughness based on observations from Disney and Epic Games
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;

    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denominator = (NdotH2 * (alpha2 - 1.0) + 1.0);
    denominator = PI * denominator * denominator;

    return alpha2 / denominator;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    // Using K from direct lighting
    float k = (r * r) / 8.0;

    float denominator = NdotV * (1.0 - k) + k;
    return NdotV / denominator;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);

    return ggx1 * ggx2;
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float ShadowCalculation(vec4 fragPosLightSpace, sampler2D shadowMap, int idx) {
    // Perspective division
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // Move coordinate to adecuate shadow map
    float tileSize = 1.0 / MAX_DIRECTIONAL_LIGHTS;
    projCoords.x = projCoords.x * tileSize + tileSize * idx;

    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r * 0.5 + 0.5;

    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // Check whether current frag pos is in shadow with given bias
    float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}

struct PBRInformation {
    vec3 position;
    vec3 N;

    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
};

vec3 PBRCalculation(PBRInformation info, vec3 V, vec3 L, vec3 F0) {
    vec3 H = normalize(V + L);

    float NDF = DistributionGGX(info.N, H, info.roughness);
    float G = GeometrySmith(info.N, V, L, info.roughness);
    vec3 F = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

    // Cook-Torrence BRDF
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(info.N, V), 0.0) * max(dot(info.N, L), 0.0) + 0.0001;// + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    // Because energy conservation, the diffuse and specular light can't be above 1.0
    vec3 kD = vec3(1.0) - kS;
    // Enforce that metallic sufraces dont refract light
    kD *= 1.0 - info.metallic;

    float NdotL = max(dot(info.N, L), 0.0);
    return (kD * info.albedo / PI + specular) * NdotL;
}

void main() {
    vec4 position = texture(uPositionMap, vTextureCoords);
    vec3 N = texture(uNormalMap, vTextureCoords).rgb;
    vec3 albedo = texture(uAlbedoMap, vTextureCoords).rgb;

    float metallic = texture(uMetallicRoughnessAOMap, vTextureCoords).r;
    float roughness = texture(uMetallicRoughnessAOMap, vTextureCoords).g;
    float ao = texture(uMetallicRoughnessAOMap, vTextureCoords).b;

    vec3 emission = texture(uEmissionMap, vTextureCoords).rgb;

    PBRInformation info;
    info.position = position.xyz;
    info.N = N;
    info.albedo = albedo;
    info.metallic = metallic;
    info.roughness = roughness;
    info.ao = ao;

    vec3 V = normalize(vCameraPosition - position.rgb);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    //
    // Points lights
    //
    for (int i = 0; i < uLightsInfo.numberPointLights; ++i) {
        PointLight light = uLightsInfo.pointLights[i];

        vec3 L = normalize(light.position.xyz - position.xyz);

        float dist = length(light.position.xyz - position.xyz);
        float attenuation = 1.0 / (dist * dist);
        vec3 radiance = (light.color * attenuation).rgb;

        Lo += PBRCalculation(info, V, L, F0) * radiance;
    }

    //
    // Directional lights
    //
    for (int i = 0; i < uLightsInfo.numberDirectionalLights; ++i) {
        DirectionalLight light = uLightsInfo.directionalLights[i];

        vec3 L = normalize(-light.direction.xyz);
        vec3 color = PBRCalculation(info, V, L, F0);

        mat4 lightSpacematrix = uShadowMappingInfo.directionalLightSpaceMatrices[light.shadowMapIdx];
        vec4 fragPosLightSpace = lightSpacematrix * position;

        if (light.shadowMapIdx >= 0 && light.shadowMapIdx < uShadowMappingInfo.numberDirectionalShadowMaps) {
            float shadow = ShadowCalculation(fragPosLightSpace, uDirectionalShadowMaps, light.shadowMapIdx);
            Lo += color * (shadow == 1.0 ? vec3(0.04) : vec3(1.0));
        } else {
            Lo += color;
        }
    }

    //
    // Ambient color
    //

    vec3 ambient = vec3(0.001) * albedo * ao;
    vec3 color = ambient + Lo;

    // Add emission
    color += emission;

    outColor = vec4(color, 1.0);
}