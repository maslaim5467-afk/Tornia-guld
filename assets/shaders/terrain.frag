#version 450
layout(location=0) in vec3 worldPosition;
layout(location=0) out vec4 outColor;
void main() {
    vec3 dx = dFdx(worldPosition);
    vec3 dy = dFdy(worldPosition);
    vec3 normal = normalize(cross(dx, dy));
    if (normal.y < 0.0) normal = -normal;
    vec3 lightDirection = normalize(vec3(-0.42, 0.82, 0.38));
    float diffuse = max(dot(normal, lightDirection), 0.0);
    float heightBlend = clamp((worldPosition.y + 10.0) / 36.0, 0.0, 1.0);
    vec3 low = vec3(0.16, 0.27, 0.13);
    vec3 high = vec3(0.34, 0.39, 0.22);
    vec3 base = mix(low, high, heightBlend);
    float lighting = 0.38 + 0.62 * diffuse;
    outColor = vec4(base * lighting, 1.0);
}
