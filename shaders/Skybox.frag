#version 450

#include "include/LightInformation.glslh"

layout (location = 0) in vec3 vTextureCoords;

layout (location = 0) out vec4 outColor;

layout (set = 1, binding = 0) uniform samplerCube uSkybox;

void main() {
    vec3 color = texture(uSkybox, vTextureCoords).rgb;
    outColor = vec4(color, 1.0);
}