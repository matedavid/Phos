#version 450

layout (binding = 0) uniform sampler2D uInputImage;
layout (binding = 1, rgba16f) restrict writeonly uniform image2D uOutputImage;

layout (push_constant) uniform BloomInformation {
    int mode;
    float threshold;
} uBloomInfo;

#define MODE_DOWNSAMPLE 0
#define MODE_UPSAMPLE   1
#define MODE_PREFILTER  2

const float KNEE = 0.1f;
const float EPSILON = 1.0e-4;

vec3 DownsampleBox13(sampler2D tex, vec2 uv, vec2 texelSize) {
    // Center
    vec3 A = texture(tex, uv).rgb;

    texelSize *= 0.5f;// Sample from center of texels

    // Inner box
    vec3 B = texture(tex, uv + texelSize * vec2(-1.0f, -1.0f)).rgb;
    vec3 C = texture(tex, uv + texelSize * vec2(-1.0f, 1.0f)).rgb;
    vec3 D = texture(tex, uv + texelSize * vec2(1.0f, 1.0f)).rgb;
    vec3 E = texture(tex, uv + texelSize * vec2(1.0f, -1.0f)).rgb;

    // Outer box
    vec3 F = texture(tex, uv + texelSize * vec2(-2.0f, -2.0f)).rgb;
    vec3 G = texture(tex, uv + texelSize * vec2(-2.0f, 0.0f)).rgb;
    vec3 H = texture(tex, uv + texelSize * vec2(0.0f, 2.0f)).rgb;
    vec3 I = texture(tex, uv + texelSize * vec2(2.0f, 2.0f)).rgb;
    vec3 J = texture(tex, uv + texelSize * vec2(2.0f, 2.0f)).rgb;
    vec3 K = texture(tex, uv + texelSize * vec2(2.0f, 0.0f)).rgb;
    vec3 L = texture(tex, uv + texelSize * vec2(-2.0f, -2.0f)).rgb;
    vec3 M = texture(tex, uv + texelSize * vec2(0.0f, -2.0f)).rgb;

    // Weights
    vec3 result = vec3(0.0);
    // Inner box
    result += (B + C + D + E) * 0.5f;
    // Bottom-left box
    result += (F + G + A + M) * 0.125f;
    // Top-left box
    result += (G + H + I + A) * 0.125f;
    // Top-right box
    result += (A + I + J + K) * 0.125f;
    // Bottom-right box
    result += (M + A + K + L) * 0.125f;

    // 4 samples each
    result *= 1.0f / 4.0f;

    return result;
}

vec3 UpsampleTent9(sampler2D tex, vec2 uv, vec2 texelSize, float radius) {
    vec4 offset = texelSize.xyxy * vec4(1.0f, 1.0f, -1.0f, 0.0f) * radius;

    // Center
    vec3 result = texture(tex, uv).rgb * 4.0f;

    result += texture(tex, uv - offset.xy).rgb;
    result += texture(tex, uv - offset.wy).rgb * 2.0;
    result += texture(tex, uv - offset.zy).rgb;

    result += texture(tex, uv + offset.zw).rgb * 2.0;
    result += texture(tex, uv + offset.xw).rgb * 2.0;

    result += texture(tex, uv + offset.zy).rgb;
    result += texture(tex, uv + offset.wy).rgb * 2.0;
    result += texture(tex, uv + offset.xy).rgb;

    return result * (1.0f / 16.0f);
}

// Quadratic color thresholding
// curve = (threshold - knee, knee * 2, 0.25 / knee)
vec3 QuadraticThreshold(vec3 color, float threshold, vec3 curve) {
    // Maximum pixel brightness
    float brightness = max(max(color.r, color.g), color.b);
    // Quadratic curve
    float rq = clamp(brightness - curve.x, 0.0, curve.y);
    rq = (rq * rq) * curve.z;
    color *= max(rq, brightness - threshold) / max(brightness, EPSILON);
    return color;
}

vec3 Prefilter(vec3 color) {
    float clampValue = 20.0f;
    color = clamp(color, vec3(0.0f), vec3(clampValue));
    color = QuadraticThreshold(color, uBloomInfo.threshold, vec3(uBloomInfo.threshold - KNEE, KNEE * 2.0f, 0.25f / KNEE));
    return color;
}

layout (local_size_x = 2, local_size_y = 2) in;

void main() {

    vec2 imgSize = vec2(imageSize(uOutputImage));

    ivec2 invocID = ivec2(gl_GlobalInvocationID);
    vec2 texCoords = vec2(float(invocID.x) / imgSize.x, float(invocID.y) / imgSize.y);
    texCoords += (1.0f / imgSize) * 0.5f;

    vec2 texSize = vec2(textureSize(uInputImage, 0));

    vec3 color = vec3(0.0f);
    if (uBloomInfo.mode == MODE_DOWNSAMPLE) {
        color = DownsampleBox13(uInputImage, texCoords, 1.0f / texSize);
    } else if (uBloomInfo.mode == MODE_UPSAMPLE) {
        float sampleScale = 2.0f;

        vec2 bloomTexSize = vec2(textureSize(uInputImage, 0));
        color = UpsampleTent9(uInputImage, texCoords, 1.0f / bloomTexSize, sampleScale);
    } else if (uBloomInfo.mode == MODE_PREFILTER) {
        color = DownsampleBox13(uInputImage, texCoords, 1.0f / texSize);
        color = Prefilter(color);
    }

    imageStore(uOutputImage, ivec2(gl_GlobalInvocationID), vec4(color, 1.0f));
}
