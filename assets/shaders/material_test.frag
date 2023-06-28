#version 450

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec2 vTextureCoords;

layout (location = 0) out vec4 ResultColor;

struct PointLight {
    vec4 color;
    vec3 position;
};

struct DirectionalLight {
    vec4 color;
    vec3 direction;
};

#define MAX_POINT_LIGHTS 10
#define MAX_DIRECTIONAL_LIGHTS 1

layout (std140, set = 0, binding = 1) uniform LightsUniformBuffer {
    PointLight pointLights[MAX_POINT_LIGHTS];
    DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];

    int numberPointLights;
    int numberDirectionalLights;
} uLightsInfo;

layout (set = 2, binding = 0) uniform sampler2D uTexture;

void main() {
    ResultColor = texture(uTexture, vTextureCoords);
}