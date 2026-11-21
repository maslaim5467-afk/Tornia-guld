#version 450

layout(location = 0) in vec3 inNormal;
layout(location = 1) in float inHeight;
layout(location = 2) in vec3 inWorldPosition;
layout(location = 0) out vec4 outColor;

void main() {
    vec3 normal = normalize(inNormal);
    vec3 lightDirection = normalize(vec3(-0.42, 0.82, 0.38));
    vec3 viewDirection = normalize(vec3(0.0, 1.0, 0.0) - inWorldPosition);
    vec3 halfDirection = normalize(lightDirection + viewDirection);

    float diffuse = max(dot(normal, lightDirection), 0.0);
    float fresnel = pow(1.0 - max(dot(normal, viewDirection), 0.0), 3.0);
    float specular = pow(max(dot(normal, halfDirection), 0.0), 52.0);
    float crest = clamp(inHeight * 0.9 + 0.5, 0.0, 1.0);

    vec3 deepWater = vec3(0.008, 0.105, 0.22);
    vec3 brightWater = vec3(0.025, 0.34, 0.49);
    vec3 water = mix(deepWater, brightWater, crest * 0.58 + fresnel * 0.18);
    water *= 0.58 + diffuse * 0.48;
    water += vec3(0.42, 0.68, 0.78) * specular * 0.72;
    outColor = vec4(water, 1.0);
}
