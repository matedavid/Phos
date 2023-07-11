#version 450

layout (location = 0) in vec2 vTextureCoords;

layout (location = 0) out vec4 outColor;

#include "include/LightInformation.glslh"

layout (set = 1, binding = 0) uniform sampler2D uBlendingTexture;

void main() {
    outColor = texture(uBlendingTexture, vTextureCoords);
}
