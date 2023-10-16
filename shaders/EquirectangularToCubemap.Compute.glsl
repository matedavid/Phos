#version 450

layout (binding = 0, rgba16f) restrict writeonly uniform imageCube uOutput;
layout (binding = 1) uniform sampler2D uEquirectangularImage;

vec3 GetCubeMapTexCoord(vec2 imageSize) {
    vec2 st = gl_GlobalInvocationID.xy / imageSize;
    vec2 uv = 2.0 * vec2(st.x, 1.0 - st.y) - vec2(1.0);

    vec3 ret;
    if (gl_GlobalInvocationID.z == 0)      ret = vec3(1.0, uv.y, -uv.x);
    else if (gl_GlobalInvocationID.z == 1) ret = vec3(-1.0, uv.y, uv.x);
    else if (gl_GlobalInvocationID.z == 2) ret = vec3(uv.x, 1.0, -uv.y);
    else if (gl_GlobalInvocationID.z == 3) ret = vec3(uv.x, -1.0, uv.y);
    else if (gl_GlobalInvocationID.z == 4) ret = vec3(uv.x, uv.y, 1.0);
    else if (gl_GlobalInvocationID.z == 5) ret = vec3(-uv.x, uv.y, -1.0);

    return normalize(ret);
}

const float PI = 3.141592;

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main() {
    vec3 cubeTC = GetCubeMapTexCoord(vec2(imageSize(uOutput)));

    // Calculate sampling coords for equirectangular texture
    // https://en.wikipedia.org/wiki/Spherical_coordinate_system#Cartesian_coordinates
    float phi = atan(cubeTC.z, cubeTC.x);
    float theta = acos(cubeTC.y);
    vec2 uv = vec2(phi / (2.0 * PI) + 0.5, theta / PI);

    vec4 color = texture(uEquirectangularImage, uv);
    imageStore(uOutput, ivec3(gl_GlobalInvocationID), color);
}
