#version 450

layout(push_constant) uniform SeaLighting {
    mat4 viewProjection;
    vec4 wave;
    vec4 cameraPosition;
    vec4 sunDirection;
} lighting;

layout(location = 0) in vec3 inNormal;
layout(location = 1) in float inHeight;
layout(location = 2) in vec3 inWorldPosition;
layout(location = 0) out vec4 outColor;

const float SEA_PI = 3.14159265359;
const float SEA_IOR = 1.333;
const float PREVIEW_WATER_DEPTH = 8.0; // assumed optical depth, metres
const vec3 WATER_F0 = vec3(0.0204);

/* Approximate clear-ocean absorption coefficients, per metre (RGB). */
const vec3 WATER_ABSORPTION = vec3(0.16, 0.055, 0.022);
const vec3 WATER_IN_SCATTER = vec3(0.004, 0.048, 0.095);

float distributionGGX(vec3 n, vec3 h, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float nh = max(dot(n, h), 0.0);
    float nh2 = nh * nh;
    float denominator = nh2 * (a2 - 1.0) + 1.0;
    return a2 / max(SEA_PI * denominator * denominator, 0.0001);
}

float geometrySchlickGGX(float nv, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return nv / max(nv * (1.0 - k) + k, 0.0001);
}

float geometrySmith(vec3 n, vec3 v, vec3 l, float roughness) {
    float nv = max(dot(n, v), 0.0);
    float nl = max(dot(n, l), 0.0);
    return geometrySchlickGGX(nv, roughness)
         * geometrySchlickGGX(nl, roughness);
}

vec3 fresnelSchlick(float cosine, vec3 f0) {
    float f = pow(clamp(1.0 - cosine, 0.0, 1.0), 5.0);
    return f0 + (1.0 - f0) * f;
}

/* Procedural sky radiance used as a lightweight environment reflection. */
vec3 sampleSky(vec3 direction, vec3 sunDirection) {
    float horizon = smoothstep(-0.18, 0.42, direction.y);
    vec3 horizonColor = vec3(0.78, 0.30, 0.12);
    vec3 zenithColor = vec3(0.018, 0.055, 0.13);
    vec3 sky = mix(horizonColor, zenithColor, horizon);

    float warmBand = exp(-abs(direction.y) * 8.0);
    sky = mix(sky, vec3(0.95, 0.43, 0.20), warmBand * 0.34);
    float sunAlignment = max(dot(direction, sunDirection), 0.0);
    float sunDisc = pow(sunAlignment, 900.0);
    float sunGlow = pow(sunAlignment, 32.0);
    sky += vec3(1.0, 0.72, 0.40) * sunDisc * 10.0;
    sky += vec3(1.0, 0.34, 0.12) * sunGlow * 0.38;
    return sky;
}

/*
 * Beer-Lambert attenuation over a preview-only water column.
 * The refracted view ray determines the optical path through an assumed
 * constant-depth slab. Since no seabed/bathymetry exists yet, this produces
 * a physically motivated color/absorption preview, not real underwater
 * visibility or bottom rendering.
 */
vec3 applyWaterAbsorption(vec3 bodyRadiance, vec3 normal, vec3 viewDirection) {
    vec3 refractedView = refract(-viewDirection, normal, 1.0 / SEA_IOR);
    float downwardCosine = max(-refractedView.y, 0.12);
    float opticalPath = min(PREVIEW_WATER_DEPTH / downwardCosine, 60.0);
    vec3 transmittance = exp(-WATER_ABSORPTION * opticalPath);
    vec3 absorbedBody = bodyRadiance * transmittance;
    vec3 inScattered = WATER_IN_SCATTER * (1.0 - transmittance);
    return absorbedBody + inScattered;
}

void main() {
    vec3 n = normalize(inNormal);
    vec3 cameraPosition = lighting.cameraPosition.xyz;
    vec3 v = normalize(cameraPosition - inWorldPosition);
    vec3 l = normalize(lighting.sunDirection.xyz);
    vec3 h = normalize(v + l);

    float nv = max(dot(n, v), 0.0);
    float nl = max(dot(n, l), 0.0);

    // Wave slope broadens highlights on steeper facets; calmer facets stay glossy.
    float slope = 1.0 - clamp(n.y, 0.0, 1.0);
    float roughness = clamp(0.095 + slope * 0.19
                          + abs(inHeight) * 0.006, 0.085, 0.32);

    vec3 f = fresnelSchlick(nv, WATER_F0);
    float d = distributionGGX(n, h, roughness);
    float g = geometrySmith(n, v, l, roughness);
    vec3 specular = (d * g * fresnelSchlick(max(dot(h, v), 0.0), WATER_F0))
                  / max(4.0 * nv * max(nl, 0.001), 0.001);

    vec3 reflectedDirection = reflect(-v, n);
    vec3 environment = sampleSky(reflectedDirection, l);

    vec3 deepWater = vec3(0.004, 0.055, 0.12);
    vec3 shallowWater = vec3(0.018, 0.22, 0.32);
    float crest = clamp(inHeight * 0.55 + 0.5, 0.0, 1.0);
    vec3 waterDiffuse = mix(deepWater, shallowWater, crest * 0.42);

    // Attenuate only the transmitted/body-water component; surface reflection
    // and the direct sun glint remain separate from the water-column absorption.
    vec3 absorbedWater = applyWaterAbsorption(waterDiffuse, n, v);

    // Low-energy hemispherical sky fill prevents the unlit side becoming black.
    float skyAmount = clamp(n.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 ambientSky = mix(vec3(0.018, 0.045, 0.075),
                          vec3(0.075, 0.15, 0.22), skyAmount);
    vec3 ambientFill = absorbedWater * ambientSky * 2.2;

    // Keep direct sunlight directional; ambient fill is independent of sun angle.
    vec3 diffuseSun = absorbedWater * vec3(1.0, 0.88, 0.72) * nl * 0.22;
    vec3 directSun = vec3(1.0, 0.82, 0.62) * specular * nl * 5.0;
    vec3 reflectedSky = environment * f;
    vec3 color = ambientFill + diffuseSun * (1.0 - f)
               + reflectedSky + directSun;

    float sunGlint = pow(max(dot(reflect(-l, n), v), 0.0), 180.0);
    color += vec3(1.0, 0.84, 0.64) * sunGlint * 0.32;

    outColor = vec4(color, 1.0);
}
