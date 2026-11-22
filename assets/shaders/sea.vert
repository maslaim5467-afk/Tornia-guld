#version 450

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform SeaCamera {
    mat4 viewProjection;
    vec4 wave; // x = elapsed seconds
    vec4 cameraPosition;
    vec4 sunDirection;
} camera;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out float outHeight;
layout(location = 2) out vec3 outWorldPosition;

const float SEA_G = 9.81;
const float SEA_GAMMA = 3.30;
const float SEA_FETCH = 50000.0;         // upwind fetch; wind comes from shared weather state
const int SEA_FREQUENCY_BINS = 8;
const int SEA_DIRECTION_BINS = 4;
const float SEA_PI = 3.14159265359;

/*
 * Fetch-limited SMB (Sverdrup-Munk-Bretschneider) sea-state estimates.
 * X = g F / U10^2
 * Hs = 0.283 U10^2/g * tanh(0.0125 X^0.42)
 * Tp = 7.54 U10/g * tanh(0.077 X^0.25)
 *
 * Empirical fetch-limited estimates, not a time-evolving wind-input model.
 * Resulting Hs/Tp parameterize the JONSWAP spectrum below.
 */
float seaSignificantWaveHeight() {
    float windSpeed=max(camera.wave.y,0.1);
    float u2=windSpeed*windSpeed;
    float dimensionlessFetch = SEA_G * SEA_FETCH / max(u2, 0.0001);
    return 0.283 * u2 / SEA_G
         * tanh(0.0125 * pow(dimensionlessFetch, 0.42));
}

float seaPeakPeriod() {
    float windSpeed=max(camera.wave.y,0.1);
    float u2=windSpeed*windSpeed;
    float dimensionlessFetch = SEA_G * SEA_FETCH / max(u2, 0.0001);
    return 7.54 * max(camera.wave.y,0.1) / SEA_G
         * tanh(0.077 * pow(dimensionlessFetch, 0.25));
}

/* Goda-normalized angular-frequency JONSWAP spectrum. */
float jonswapSpectrum(float omega, float significantHeight, float peakPeriod) {
    float omegaPeak = 2.0 * SEA_PI / peakPeriod;
    float sigma = (omega <= omegaPeak) ? 0.07 : 0.09;
    float betaJ = 0.0624 * (1.094 - 0.01915 * log(SEA_GAMMA))
                / (0.230 + 0.0336 * SEA_GAMMA
                   - 0.185 / (1.9 + SEA_GAMMA));
    float ratio = omegaPeak / max(omega, 0.0001);
    float peakDistance = (omega - omegaPeak) / (sigma * omegaPeak);
    float peakShape = exp(-0.5 * peakDistance * peakDistance);
    return betaJ * significantHeight * significantHeight
         * pow(omegaPeak, 4.0) / pow(max(omega, 0.0001), 5.0)
         * exp(-1.25 * pow(ratio, 4.0))
         * pow(SEA_GAMMA, peakShape);
}

/* Cosine-2s directional spreading, discretely normalized per frequency. */
float directionalShape(float angleOffset, float spreadingExponent) {
    float c = max(cos(0.5 * angleOffset), 0.0);
    return pow(c, 2.0 * spreadingExponent);
}

void main() {
    float t = camera.wave.x;
    float significantHeight = seaSignificantWaveHeight();
    float peakPeriod = seaPeakPeriod();
    float omegaPeak = 2.0 * SEA_PI / peakPeriod;
    float deltaOmega = 0.25 * omegaPeak;
    float variance = 0.0;
    float steepnessSum = 0.0;

    for (int fi = 0; fi < SEA_FREQUENCY_BINS; ++fi) {
        float omega = (0.625 + 0.25 * float(fi)) * omegaPeak;
        float binVariance = max(
            jonswapSpectrum(omega, significantHeight, peakPeriod)
            * deltaOmega, 0.0);
        float amplitude = sqrt(2.0 * binVariance);
        float waveNumber = omega * omega / SEA_G;
        variance += binVariance;
        steepnessSum += waveNumber * amplitude;
    }

    // Hs ~= 4 * sqrt(m0): normalize sampled energy to the SMB sea state.
    float targetVariance = significantHeight * significantHeight / 16.0;
    float energyScale = (variance > 0.000001)
                      ? sqrt(targetVariance / variance) : 0.0;
    float choppiness = min(0.65,
        0.85 / max(steepnessSum * energyScale, 0.0001));

    vec3 displaced = inPosition;
    vec3 tangentX = vec3(1.0, 0.0, 0.0);
    vec3 tangentZ = vec3(0.0, 0.0, 1.0);
    const float directionOffsets[SEA_DIRECTION_BINS] =
        float[SEA_DIRECTION_BINS](-0.90, -0.30, 0.30, 0.90);

    for (int fi = 0; fi < SEA_FREQUENCY_BINS; ++fi) {
        float frequencyIndex = float(fi);
        float omega = (0.625 + 0.25 * frequencyIndex) * omegaPeak;
        float spectralDensity = jonswapSpectrum(
            omega, significantHeight, peakPeriod);
        float binVariance = max(spectralDensity * deltaOmega, 0.0);
        float waveNumber = omega * omega / SEA_G;
        float angularFrequency = sqrt(SEA_G * waveNumber);

        // Long waves are more aligned with wind; shorter waves spread more.
        float frequencyBlend = clamp(
            (omega / omegaPeak - 0.625) / 1.75, 0.0, 1.0);
        float spreadingExponent = mix(8.0, 2.0, frequencyBlend);

        float directionalSum = 0.0;
        for (int di = 0; di < SEA_DIRECTION_BINS; ++di) {
            directionalSum += directionalShape(
                directionOffsets[di], spreadingExponent);
        }

        // Split, but do not change, each frequency bin's total variance.
        for (int di = 0; di < SEA_DIRECTION_BINS; ++di) {
            float offset = directionOffsets[di];
            float rawWeight = directionalShape(offset, spreadingExponent);
            float directionalWeight = rawWeight / max(directionalSum, 0.0001);
            float amplitude = sqrt(2.0 * binVariance
                                 * directionalWeight) * energyScale;

            float angle = camera.wave.z + offset;
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
