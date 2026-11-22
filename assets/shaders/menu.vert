#version 450
layout(location=0) in vec2 p;
layout(location=1) in vec4 c;
layout(location=2) in vec2 uvIn;
layout(location=3) in float texturedIn;
layout(location=0) out vec4 color;
layout(location=1) out vec2 uv;
layout(location=2) out float textured;
void main() {
    gl_Position = vec4(p, 0.0, 1.0);
    color = c;
    uv = uvIn;
    textured = texturedIn;
}
