#version 450

layout (binding = 0) uniform sampler2D uInputImage;
layout (binding = 1, rgba16f) restrict writeonly uniform image2D uOutputImage;

layout (local_size_x = 2, local_size_y = 2) in;

void main() {
    vec2 imgSize = vec2(imageSize(uOutputImage));
    ivec2 invocationID = ivec2(gl_GlobalInvocationID);

    vec2 texCoords = vec2(float(invocationID.x) / imgSize.x, float(invocationID.y) / imgSize.y);
    texCoords += (1.0f / imgSize) * 0.5f;

    vec3 color = texture(uInputImage, texCoords).rgb;
    imageStore(uOutputImage, invocationID, vec4(color, 1.0f));
}
