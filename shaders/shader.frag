#version 450

layout (location = 0) out vec4 ResultColor;

layout (location = 0) in vec3 FragColor;
layout (location = 1) in vec2 fragTexCoord;

layout (binding = 1) uniform sampler2D texSampler;

void main() {
    ResultColor = texture(texSampler, fragTexCoord);
}