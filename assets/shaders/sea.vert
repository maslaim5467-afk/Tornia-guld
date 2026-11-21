#version 450

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform SeaCamera {
    mat4 viewProjection;
    vec4 wave; // x = elapsed seconds
} camera;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out float outHeight;
layout(location = 2) out vec3 outWorldPosition;

/*
 * Gerstner component.
 *
 * phase = k * dot(direction, position) - omega * time
 * horizontal displacement = Q * A * direction * cos(phase)
 * vertical displacement   = A * sin(phase)
 *
 * The partial derivatives are accumulated analytically so the fragment
 * shader receives a geometric normal for the displaced parametric surface.
 */
void addGerstnerWave(
    inout vec3 position,
    inout vec3 tangentX,
    inout vec3 tangentZ,
    vec2 direction,
    float amplitude,
    float waveNumber,
    float angularFrequency,
    float steepness,
    float timeSeconds
) {
    float phase = waveNumber * dot(direction, inPosition.xz)
                - angularFrequency * timeSeconds;
    float s = sin(phase);
    float c = cos(phase);
    float qa = steepness * amplitude;
    float ak = amplitude * waveNumber;
    float qak = qa * waveNumber;

    position.x += qa * direction.x * c;
    position.y += amplitude * s;
    position.z += qa * direction.y * c;

    tangentX.x -= qak * direction.x * direction.x * s;
    tangentX.y += ak * direction.x * c;
    tangentX.z -= qak * direction.x * direction.y * s;

    tangentZ.x -= qak * direction.x * direction.y * s;
    tangentZ.y += ak * direction.y * c;
    tangentZ.z -= qak * direction.y * direction.y * s;
}

void main() {
    float t = camera.wave.x;

    vec3 displaced = inPosition;
    vec3 tangentX = vec3(1.0, 0.0, 0.0);
    vec3 tangentZ = vec3(0.0, 0.0, 1.0);

    // Long swell: high amplitude and long wavelength.
    addGerstnerWave(displaced, tangentX, tangentZ,
        normalize(vec2(0.92, 0.39)),
        0.48, 0.449, sqrt(9.81 * 0.449) * 0.72, 0.65, t);

    // Crossing secondary swell.
    addGerstnerWave(displaced, tangentX, tangentZ,
        normalize(vec2(-0.28, 0.96)),
        0.26, 0.785, sqrt(9.81 * 0.785) * 0.78, 0.58, t);

    // Medium chop.
    addGerstnerWave(displaced, tangentX, tangentZ,
        normalize(vec2(0.72, -0.69)),
        0.13, 1.396, sqrt(9.81 * 1.396) * 0.82, 0.42, t);

    // Fine surface detail.
    addGerstnerWave(displaced, tangentX, tangentZ,
        normalize(vec2(-0.86, -0.51)),
        0.07, 2.513, sqrt(9.81 * 2.513) * 0.86, 0.28, t);

    vec3 normal = normalize(cross(tangentZ, tangentX));

    outNormal = normal;
    outHeight = displaced.y;
    outWorldPosition = displaced;
    gl_Position = camera.viewProjection * vec4(displaced, 1.0);
}
