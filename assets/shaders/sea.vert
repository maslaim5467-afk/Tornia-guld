#version 450

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform SeaCamera {
    mat4 viewProjection;
    vec4 wave; // x = elapsed seconds
} camera;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out float outHeight;
layout(location = 2) out vec3 outWorldPosition;

void main() {
    float t = camera.wave.x;
    float x = inPosition.x;
    float z = inPosition.z;

    float p1 = x * 0.48 + z * 0.12 + t * 1.05;
    float p2 = z * 0.72 - x * 0.10 - t * 0.82;
    float p3 = (x + z) * 0.34 + t * 0.58;
    float p4 = (x - z) * 0.88 - t * 1.32;

    float height = 0.42 * sin(p1)
                 + 0.25 * sin(p2)
                 + 0.14 * sin(p3)
                 + 0.075 * sin(p4);

    float dx = 0.42 * 0.48 * cos(p1)
             - 0.25 * 0.10 * cos(p2)
             + 0.14 * 0.34 * cos(p3)
             + 0.075 * 0.88 * cos(p4);
    float dz = 0.42 * 0.12 * cos(p1)
             + 0.25 * 0.72 * cos(p2)
             + 0.14 * 0.34 * cos(p3)
             - 0.075 * 0.88 * cos(p4);

    outNormal = normalize(vec3(-dx, 1.0, -dz));
    outHeight = height;
    outWorldPosition = vec3(x, height, z);

    gl_Position = camera.viewProjection * vec4(outWorldPosition, 1.0);
}
