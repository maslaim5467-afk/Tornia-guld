#version 450

layout(location = 0) in vec3 inNormal;
layout(location = 1) in float inHeight;
layout(location = 0) out vec4 outColor;

void main() {
    vec3 normal = normalize(inNormal);
    vec3 lightDirection = normalize(vec3(-0.35, 0.82, 0.45));
    float diffuse = max(dot(normal, lightDirection), 0.0);
    float crest = clamp(inHeight * 1.8 + 0.5, 0.0, 1.0);
    float lighting = 0.62 + diffuse * 0.32 + crest * 0.06;

    vec3 deepWater = vec3(0.012, 0.16, 0.31);
    vec3 brightWater = vec3(0.035, 0.39, 0.58);
    vec3 water = mix(deepWater, brightWater, crest * 0.45);
    outColor = vec4(water * lighting, 1.0);
}
