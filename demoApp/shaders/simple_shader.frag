#version 450

layout (location = 0) in vec3 fragColor;

// Layout qualifier; multiple output location (we are using 0 here)
layout (location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0); // R,G,B,A
}