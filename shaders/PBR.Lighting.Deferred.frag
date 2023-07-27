#version 450

layout (location = 0) in vec2 vTextureCoords;
layout (location = 1) in vec3 vCameraPosition;
layout (location = 2) in mat4 vLightSpaceMatrix;

layout (location = 0) out vec4 outColor;

#include "include/LightInformation.glslh"

layout (set = 1, binding = 0) uniform sampler2D uPositionMap;
layout (set = 1, binding = 1) uniform sampler2D uNormalMap;
layout (set = 1, binding = 2) uniform sampler2D uAlbedoMap;
layout (set = 1, binding = 3) uniform sampler2D uMetallicRoughnessAOMap;
layout (set = 1, binding = 4) uniform sampler2D uShadowMap;

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

float ShadowCalculation(vec4 fragPosLightSpace) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(uShadowMap, projCoords.xy).r * 0.5 + 0.5;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // check whether current frag pos is in shadow
    float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}

void main() {
    vec4 position = texture(uPositionMap, vTextureCoords);
    vec3 N = texture(uNormalMap, vTextureCoords).rgb;
    vec3 albedo = pow(texture(uAlbedoMap, vTextureCoords).rgb, vec3(2.2));

    float metallic = texture(uMetallicRoughnessAOMap, vTextureCoords).r;
    float roughness = texture(uMetallicRoughnessAOMap, vTextureCoords).g;
    float ao = texture(uMetallicRoughnessAOMap, vTextureCoords).b;

    vec3 V = normalize(vCameraPosition - position.rgb);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < uLightsInfo.numberPointLights; ++i) {
        PointLight light = uLightsInfo.pointLights[i];

        vec3 L = normalize(light.position - position.rgb);
        vec3 H = normalize(V + L);

        float dist = length(light.position - position.rgb);
        float attenuation = 1.0 / (dist * dist);
        vec3 radiance = (light.color * attenuation).rgb;

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

        // Cook-Torrence BRDF
        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        // Because energy conservation, the diffuse and specular light can't be above 1.0
        vec3 kD = vec3(1.0) - kS;
        // Enforce that metallic sufraces dont refract light
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    //
    // Ambient color
    //

    float shadow = ShadowCalculation(vLightSpaceMatrix * position);

    vec3 ambient = vec3(0.03) * albedo * ao * (shadow == 1.0 ? vec3(0.04) : vec3(1.0));
    vec3 color = ambient + (1.0 - shadow) * Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, 1.0);
}