#version 450
layout(set=0,binding=0) uniform sampler2D terrainAtlas;
layout(location=0) in vec4 color;
layout(location=1) in vec2 uv;
layout(location=2) in float textured;
layout(location=0) out vec4 outColor;
void main() {
    vec4 sampled = texture(terrainAtlas, uv);
    outColor = mix(color, sampled * color, textured);
}
