#version 450

layout (location = 0) in vec2 vTextureCoords;

layout (location = 0) out vec4 outColor;

#include "include/LightInformation.glslh"

layout (set = 1, binding = 0) uniform sampler2D uBlendingTexture;

void main() {
    // Convert back to linear because reading from UNORM image with sRGB conversion applied, but writing to sRGB target.
    vec3 color = pow(texture(uBlendingTexture, vTextureCoords).rgb, vec3(2.2));

    outColor = vec4(color, texture(uBlendingTexture, vTextureCoords).a);
}
