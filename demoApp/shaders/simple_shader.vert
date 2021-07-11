#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(push_constant) uniform Push {
    mat4 model;
    mat4 view;
} push;

layout (binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

layout(location = 0) out vec3 fragColor;

void main() {
//    gl_Position = vec4(push.transform * position + push.offset, 0.0, 1.0); // x, y, z, scale?
//    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(position, 1.0);
    gl_Position = ubo.projection * push.view * push.model * vec4(position, 1.0);
    fragColor = color;
}