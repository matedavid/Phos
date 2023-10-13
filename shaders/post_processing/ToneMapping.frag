#version 450

layout (location = 0) in vec2 vTextureCoords;

layout (location = 0) out vec4 outColor;

#include "../include/LightInformation.glslh"

layout (push_constant) uniform ToneMappingConfig {
    int bloomEnabled;
} uConfig;

layout (set = 1, binding = 0) uniform sampler2D uResultTexture;
layout (set = 1, binding = 1) uniform sampler2D uBloomTexture;

vec3 ReinhardToneMapping(vec3 color) {
    return color / (color + vec3(1.0f));
}

vec3 GammaCorrection(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));
}

void main() {
    vec3 color = texture(uResultTexture, vTextureCoords).rgb;

    // Add Bloom Contribution
    if (uConfig.bloomEnabled == 1) {
        color += texture(uBloomTexture, vTextureCoords).rgb;
    }

    // Tone Mapping
    color = ReinhardToneMapping(color);

    // Gamma Correction
    color = GammaCorrection(color);

    outColor = vec4(color, 1.0f);
}
