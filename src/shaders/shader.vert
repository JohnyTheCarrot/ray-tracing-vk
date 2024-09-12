#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in mat4 instanceModelMat;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = ubo.proj * ubo.view * instanceModelMat * vec4(inPosition, 1.0);
    fragColor = vec3(0.5, 0.5, 0.5);
}
