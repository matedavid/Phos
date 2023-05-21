#version 450

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec4 vColor;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormals;
layout (location = 2) out vec4 outColorSpecular;

void main() {
    outPosition = vPosition;
    outNormals.xyz = vNormal;
    outColorSpecular.xyz = vColor.xyz;
    outColorSpecular.w = 1.0f;
}