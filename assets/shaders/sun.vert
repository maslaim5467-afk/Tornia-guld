#version 450
layout(location=0) in vec2 position;
layout(location=1) in vec4 colorIn;
layout(location=3) in float depthIn;
layout(location=0) out vec4 color;
void main() {
    gl_Position = vec4(position, depthIn, 1.0);
    color = colorIn;
}
