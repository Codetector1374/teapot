#version 450

// Layout qualifier; multiple output location (we are using 0 here)
layout (location = 0) out vec4 outColor;

layout (push_constant) uniform Push {
    mat2 transform;
    vec2 offset;
    vec3 color;
} push;

void main() {
    outColor = vec4(push.color, 1.0); // R,G,B,A
}