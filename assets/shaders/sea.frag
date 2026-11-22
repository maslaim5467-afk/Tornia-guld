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

/*
 * Compact single-scattering dawn atmosphere for the procedural environment.
 * The optical-depth approximation is intentionally bounded for mobile GPUs;
 * it captures the long-path red shift near the horizon without a full
 * volumetric atmosphere or aerial-perspective pass.
 */
vec3 atmosphericTransmittance(float sunElevation) {
    float elevation = max(sunElevation, 0.0);
    float airMass = 1.0 / max(elevation + 0.055, 0.075);
    airMass = min(airMass, 8.0);

    // Relative RGB extinction: blue is scattered out of the direct solar beam
    // more strongly than red along the long low-angle path at sunrise.
    vec3 extinction = vec3(0.035, 0.105, 0.24);
    return exp(-extinction * airMass);
}

/* Procedural sky radiance used as a lightweight environment reflection. */
vec3 sampleSky(vec3 direction, vec3 sunDirection) {
    vec3 viewDir = normalize(direction);
    vec3 sunDir = normalize(sunDirection);
    float elevation = max(sunDir.y, 0.0);
    float horizon = smoothstep(-0.20, 0.48, viewDir.y);

    vec3 zenithColor = vec3(0.012, 0.035, 0.095);
    vec3 horizonBase = vec3(0.16, 0.075, 0.075);
    vec3 sky = mix(horizonBase, zenithColor, horizon);

    // A broad warm horizon belt represents the strongly scattered dawn light.
    float horizonGlow = exp(-abs(viewDir.y) * 10.0);
    float sunSide = 0.55 + 0.45 * max(dot(viewDir, sunDir), 0.0);
    sky += vec3(0.42, 0.105, 0.028) * horizonGlow * sunSide;

    // Rayleigh-like angular response plus a broader forward Mie lobe.
    float cosTheta = clamp(dot(viewDir, sunDir), -1.0, 1.0);
    float rayleighPhase = 0.0597 * (1.0 + cosTheta * cosTheta);
    const float mieG = 0.76;
    float mieDenom = max(1.0 + mieG * mieG - 2.0 * mieG * cosTheta, 0.025);
    float miePhase = 0.018 * (1.0 - mieG * mieG)
                   / (mieDenom * sqrt(mieDenom));
    vec3 transmittance = atmosphericTransmittance(elevation);
    vec3 scatteredSun = vec3(1.0, 0.57, 0.30) * transmittance;
    sky += scatteredSun * (rayleighPhase * 5.0 + miePhase * 0.8);

    // Finite angular solar disc (~0.27 degrees) with a soft optical aureole.
    float sunCos = cos(0.00465);
    float disc = smoothstep(sunCos, cos(0.0036), cosTheta);
    float aureole = pow(max(cosTheta, 0.0), 180.0);
    vec3 solarRadiance = vec3(1.0, 0.76, 0.48) * transmittance;
    sky += solarRadiance * disc * 18.0;
    sky += solarRadiance * aureole * 0.32;
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

    // Low-angle solar beam is spectrally reddened by the longer dawn air path.
    // Ambient fill remains independent of the direct solar elevation.
    vec3 solarTransmission = atmosphericTransmittance(l.y);
    vec3 solarColor = vec3(1.0, 0.91, 0.76) * solarTransmission;
    float solarElevation = smoothstep(-0.015, 0.12, l.y);
    vec3 diffuseSun = absorbedWater * solarColor * nl
                    * (0.30 + 0.70 * solarElevation);
    vec3 directSun = solarColor * specular * nl * 7.0;
    vec3 reflectedSky = environment * f;
    vec3 color = ambientFill + diffuseSun * (1.0 - f)
               + reflectedSky + directSun;

    // Broaden the reflected solar streak slightly on rougher wave facets.
    float sunAlignment = max(dot(reflect(-l, n), v), 0.0);
    float glintExponent = mix(240.0, 90.0, roughness);
    float sunGlint = pow(sunAlignment, glintExponent);
    color += solarColor * vec3(1.0, 0.84, 0.66) * sunGlint * 0.48;

    outColor = vec4(color, 1.0);
}
