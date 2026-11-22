#version 450
layout(location=0) in vec3 inPosition;
layout(push_constant) uniform TerrainPush { mat4 viewProjection; } pc;
layout(location=0) out vec3 worldPosition;
void main() {
    worldPosition = inPosition;
    gl_Position = pc.viewProjection * vec4(inPosition, 1.0);
}
