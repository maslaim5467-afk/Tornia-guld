#version 450
layout(set=0,binding=0) uniform sampler2D terrainAtlas;
layout(set=0,binding=1) uniform sampler2D spriteSheet;
layout(location=0) in vec4 color;
layout(location=1) in vec2 uv;
layout(location=2) in float textured;
layout(location=0) out vec4 outColor;
void main() {
    vec4 sampled = textured > 1.5 ? texture(spriteSheet, uv) : texture(terrainAtlas, uv);
    outColor = mix(color, sampled * color, step(0.5, textured));
}
