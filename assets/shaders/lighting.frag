#version 450

layout (location = 0) out vec4 ResultColor;

layout (std140, set = 1, binding = 0) uniform LightsUniformBuffer {
    vec4 positions[10];
    vec4 colors[10];
    int count;
};

layout (set = 1, binding = 1) uniform sampler2D uPositionMap;
layout (set = 1, binding = 2) uniform sampler2D uNormalMap;
layout (set = 1, binding = 3) uniform sampler2D uColorSpecularMap;

layout (location = 0) in vec2 vTexCoordinates;
layout (location = 1) in vec3 vCameraPosition;

vec3 CalcPointLight(int idx, vec3 normal, vec3 fragPos, vec3 viewDir, vec4 Color) {
    vec3 lightDir = normalize(positions[idx].xyz - fragPos);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0f);

    // attenuation
    float distance = length(positions[idx].xyz - fragPos);
    float attenuation = 1.0 / (1.0f + 0.09 * distance + 0.032f * (distance * distance));

    // combine results
    vec3 ambient = vec3(0.01f) * Color.xyz;
    vec3 diffuse = colors[idx].zyx * diff * Color.xyz; // .zyx because framebuffer format is BGR
    vec3 specular = vec3(1.0f) * spec * vec3(Color.w);

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

void main() {
    // Texture values
    vec4 position = texture(uPositionMap, vTexCoordinates);
    vec3 normal = texture(uNormalMap, vTexCoordinates).xyz;
    vec4 color = texture(uColorSpecularMap, vTexCoordinates);

    // properties
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(vCameraPosition - position.xyz);

    vec3 result = vec3(0.0f);
    for (int i = 0; i < count; i++) {
        result += CalcPointLight(i, norm, vec3(vTexCoordinates, 1.0f), viewDir, color);
    }

    ResultColor = vec4(result, 1.0f);
}