#version 450

layout (location = 0) in vec2 vTextureCoords;
layout (location = 0) out vec4 ResultColor;

layout (binding = 0) uniform ColorUniformBuffer {
    vec4 Color;
};
layout (binding = 1) uniform sampler2D Texture;


void main() {
    ResultColor = texture(Texture, vTextureCoords) * Color;
}