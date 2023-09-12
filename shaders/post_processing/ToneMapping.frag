#version 450

layout (location = 0) in vec2 vTextureCoords;

layout (location = 0) out vec4 outColor;

#include "../include/LightInformation.glslh"

layout (set = 1, binding = 0) uniform sampler2D uToneMappingTexture;

vec3 ReinhardToneMapping(vec3 color) {
    return color / (color + vec3(1.0f));
}

vec3 GammaCorrection(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));
}

void main() {
    vec3 color = texture(uToneMappingTexture, vTextureCoords).rgb;

    // Tone Mapping
    color = ReinhardToneMapping(color);

    // Gamma Correction
    color = GammaCorrection(color);

    outColor = vec4(color, 1.0f);
}
