#version 450

// Layout qualifier; multiple output location (we are using 0 here)
layout (location = 0) out vec4 outColor;

void main() {
    outColor = vec4(0.4, .4, .4, 1.0); // R,G,B,A
}