#version 450

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec4 vColor;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormals;
layout (location = 2) out vec4 outColorSpecular;

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
    int numberPointLights;
    int numberDirectionalLights;

    PointLight pointLights[MAX_POINT_LIGHTS];
    DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
} uLightsInfo;

void main() {
    outPosition = vPosition;
    outNormals = vec4(0.0f);
    outColorSpecular = vec4(vColor.xyz, 1.0f);
}