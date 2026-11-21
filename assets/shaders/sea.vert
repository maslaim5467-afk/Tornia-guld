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
const float SEA_WIND_DIRECTION = 0.35; // radians, preview wind direction
const int SEA_FREQUENCY_BINS = 8;
const int SEA_DIRECTION_BINS = 4;
const float SEA_PI = 3.14159265359;

/*
 * Goda-normalized angular-frequency JONSWAP spectrum.
 * S(omega) has units of variance per angular-frequency interval.
 */
float jonswapSpectrum(float omega) {
    float omegaPeak = 2.0 * SEA_PI / SEA_TP;
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

/*
 * Cosine-2s directional spreading:
 * D(theta) proportional to cos^(2s)((theta - windDirection) / 2)
 * inside the forward half-plane. We normalize the discrete directional
 * quadrature below, so the sampled weights sum to one for each frequency.
 */
float directionalShape(float angleOffset, float spreadingExponent) {
    float c = max(cos(0.5 * angleOffset), 0.0);
    return pow(c, 2.0 * spreadingExponent);
}

void main() {
    float t = camera.wave.x;
    float omegaPeak = 2.0 * SEA_PI / SEA_TP;
    float deltaOmega = 0.25 * omegaPeak;
    float variance = 0.0;
    float steepnessSum = 0.0;

    // Integrate the sampled JONSWAP frequency spectrum.
    for (int fi = 0; fi < SEA_FREQUENCY_BINS; ++fi) {
        float omega = (0.625 + 0.25 * float(fi)) * omegaPeak;
        float binVariance = max(jonswapSpectrum(omega) * deltaOmega, 0.0);
        float amplitude = sqrt(2.0 * binVariance);
        float waveNumber = omega * omega / SEA_G;
        variance += binVariance;
        steepnessSum += waveNumber * amplitude;
    }

    // Hs ~= 4 * sqrt(m0): normalize the discrete frequency energy.
    float targetVariance = SEA_HS * SEA_HS / 16.0;
    float energyScale = (variance > 0.000001)
                      ? sqrt(targetVariance / variance) : 0.0;
    float choppiness = min(0.65,
        0.85 / max(steepnessSum * energyScale, 0.0001));

    vec3 displaced = inPosition;
    vec3 tangentX = vec3(1.0, 0.0, 0.0);
    vec3 tangentZ = vec3(0.0, 0.0, 1.0);

    // Direction samples around the prevailing wind, in radians.
    const float directionOffsets[SEA_DIRECTION_BINS] =
        float[SEA_DIRECTION_BINS](-0.90, -0.30, 0.30, 0.90);

    for (int fi = 0; fi < SEA_FREQUENCY_BINS; ++fi) {
        float frequencyIndex = float(fi);
        float omega = (0.625 + 0.25 * frequencyIndex) * omegaPeak;
        float spectralDensity = jonswapSpectrum(omega);
        float binVariance = max(spectralDensity * deltaOmega, 0.0);
        float waveNumber = omega * omega / SEA_G;
        float angularFrequency = sqrt(SEA_G * waveNumber);

        // Longer waves are more directionally concentrated; short waves spread more.
        float frequencyBlend = clamp(
            (omega / omegaPeak - 0.625) / 1.75, 0.0, 1.0);
        float spreadingExponent = mix(8.0, 2.0, frequencyBlend);

        float directionalSum = 0.0;
        for (int di = 0; di < SEA_DIRECTION_BINS; ++di) {
            directionalSum += directionalShape(
                directionOffsets[di], spreadingExponent);
        }

        // Split each frequency bin's variance among directions without
        // changing its total energy: sum(directionVariance) == binVariance.
        for (int di = 0; di < SEA_DIRECTION_BINS; ++di) {
            float offset = directionOffsets[di];
            float rawWeight = directionalShape(offset, spreadingExponent);
            float directionalWeight = rawWeight / max(directionalSum, 0.0001);
            float amplitude = sqrt(2.0 * binVariance
                                 * directionalWeight) * energyScale;

            float angle = SEA_WIND_DIRECTION + offset;
            vec2 direction = vec2(cos(angle), sin(angle));
            float phase = waveNumber * dot(direction, inPosition.xz)
                        - angularFrequency * t
                        + frequencyIndex * 1.6180339
                        + float(di) * 2.3999632;

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
    }

    outNormal = normalize(cross(tangentZ, tangentX));
    outHeight = displaced.y;
    outWorldPosition = displaced;
    gl_Position = camera.viewProjection * vec4(displaced, 1.0);
}
