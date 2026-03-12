#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(push_constant) uniform PushConstants {
    mat4 view;
    mat4 projection;
    mat4 model;
} push;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = push.projection * push.view * push.model * vec4(inPosition, 1.0);
    fragColor = inColor;
}