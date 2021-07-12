#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

// Layout qualifier; multiple output location (we are using 0 here)
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
    mat4 model;
    mat4 view;
} push;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
//    outColor = vec4(push.color, 1.0); // R,G,B,A
    outColor = texture(texSampler, fragTexCoord);
}