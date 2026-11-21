#version 450

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform SeaCamera {
    mat4 viewProjection;
    vec4 wave; // x = elapsed seconds
} camera;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out float outHeight;
layout(location = 2) out vec3 outWorldPosition;

const float SEA_G = 9.81;
const float SEA_HS = 1.20;       // significant wave height, world units
const float SEA_TP = 4.50;       // spectral peak period, seconds
const float SEA_GAMMA = 3.30;    // JONSWAP peak enhancement
const int SEA_BINS = 8;

/*
 * Goda-normalized angular-frequency JONSWAP spectrum.
 * S(omega) has units of variance per angular-frequency interval.
 */
float jonswapSpectrum(float omega) {
    float omegaPeak = 6.28318530718 / SEA_TP;
    float sigma = (omega <= omegaPeak) ? 0.07 : 0.09;
    float betaJ = 0.0624 * (1.094 - 0.01915 * log(SEA_GAMMA))
                / (0.230 + 0.0336 * SEA_GAMMA
                   - 0.185 / (1.9 + SEA_GAMMA));
    float ratio = omegaPeak / max(omega, 0.0001);
    float peakDistance = (omega - omegaPeak)
                       / (sigma * omegaPeak);
    float peakShape = exp(-0.5 * peakDistance * peakDistance);
    return betaJ * SEA_HS * SEA_HS
         * pow(omegaPeak, 4.0) / pow(max(omega, 0.0001), 5.0)
         * exp(-1.25 * pow(ratio, 4.0))
         * pow(SEA_GAMMA, peakShape);
}

void main() {
    float t = camera.wave.x;
    float omegaPeak = 6.28318530718 / SEA_TP;
    float deltaOmega = 0.25 * omegaPeak;
    float variance = 0.0;
    float steepnessSum = 0.0;

    // First pass: integrate the sampled JONSWAP energy and estimate steepness.
    for (int i = 0; i < SEA_BINS; ++i) {
        float omega = (0.625 + 0.25 * float(i)) * omegaPeak;
        float spectralDensity = jonswapSpectrum(omega);
        float binVariance = max(spectralDensity * deltaOmega, 0.0);
        float amplitude = sqrt(2.0 * binVariance);
        float waveNumber = omega * omega / SEA_G;
        variance += binVariance;
        steepnessSum += waveNumber * amplitude;
    }

    // Normalize the discrete spectrum to the requested significant wave height:
    // Hs ~= 4 * sqrt(m0), where m0 is the surface-elevation variance.
    float targetVariance = SEA_HS * SEA_HS / 16.0;
    float energyScale = (variance > 0.000001)
                      ? sqrt(targetVariance / variance) : 0.0;
    float choppiness = min(0.65, 0.85 / max(steepnessSum * energyScale, 0.0001));

    vec3 displaced = inPosition;
    vec3 tangentX = vec3(1.0, 0.0, 0.0);
    vec3 tangentZ = vec3(0.0, 0.0, 1.0);

    // Second pass: convert spectral bins into directional Gerstner components.
    // JONSWAP is a frequency spectrum; this fixed directional fan is a
    // lightweight directional approximation, not a full 2-D directional spectrum.
    for (int i = 0; i < SEA_BINS; ++i) {
        float fi = float(i);
        float omega = (0.625 + 0.25 * fi) * omegaPeak;
        float spectralDensity = jonswapSpectrum(omega);
        float amplitude = sqrt(max(2.0 * spectralDensity * deltaOmega, 0.0))
                        * energyScale;
        float waveNumber = omega * omega / SEA_G;
        float phaseSpeed = sqrt(SEA_G * waveNumber);

        float angle = -0.42 + 0.12 * fi;
        vec2 direction = normalize(vec2(cos(angle), sin(angle)));
        float phase = waveNumber * dot(direction, inPosition.xz)
                    - phaseSpeed * t
                    + fi * 1.6180339;

        float s = sin(phase);
        float c = cos(phase);
        float qa = choppiness * amplitude;
        float ak = amplitude * waveNumber;
        float qak = qa * waveNumber;

        displaced.x += qa * direction.x * c;
        displaced.y += amplitude * s;
        displaced.z += qa * direction.y * c;

        tangentX.x -= qak * direction.x * direction.x * s;
        tangentX.y += ak * direction.x * c;
        tangentX.z -= qak * direction.x * direction.y * s;

        tangentZ.x -= qak * direction.x * direction.y * s;
        tangentZ.y += ak * direction.y * c;
        tangentZ.z -= qak * direction.y * direction.y * s;
    }

    vec3 normal = normalize(cross(tangentZ, tangentX));
    outNormal = normal;
    outHeight = displaced.y;
    outWorldPosition = displaced;
    gl_Position = camera.viewProjection * vec4(displaced, 1.0);
}
