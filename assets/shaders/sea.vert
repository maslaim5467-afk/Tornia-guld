#version 450

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform SeaCamera {
    mat4 viewProjection;
    vec4 wave; // x = elapsed seconds
} camera;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out float outHeight;

void main() {
    float t = camera.wave.x;
    float x = inPosition.x;
    float z = inPosition.z;

    float phaseA = x * 0.55 + t * 1.10;
    float phaseB = z * 0.75 - t * 0.80;
    float phaseC = (x + z) * 0.32 + t * 0.60;

    float height = 0.18 * sin(phaseA)
                 + 0.11 * sin(phaseB)
                 + 0.07 * sin(phaseC);

    float slopeX = 0.099 * cos(phaseA) + 0.0224 * cos(phaseC);
    float slopeZ = 0.0825 * cos(phaseB) + 0.0224 * cos(phaseC);

    outNormal = normalize(vec3(-slopeX, 1.0, -slopeZ));
    outHeight = height;

    vec3 displaced = vec3(x, height, z);
    gl_Position = camera.viewProjection * vec4(displaced, 1.0);
}
