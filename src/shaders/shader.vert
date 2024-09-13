#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 uv;
layout(location = 3) in mat4 instanceModelMat;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUv;
layout(binding = 1) uniform sampler2D texSampler;

void main() {
    gl_Position = ubo.proj * ubo.view * instanceModelMat * vec4(inPosition, 1.0);

    // Fragment shader part
    // Calculate the world position of the vertex
    vec4 worldPos = instanceModelMat * vec4(inPosition, 1.0);

    // Transform the world position into view space
    vec4 viewPos = ubo.view * worldPos;

    // The distance from the camera (which is at the origin in view space)
    float distanceFromCamera = length(viewPos.xyz);

    // Define the maximum distance at which the color will turn black
    float maxDistance = 5000.0;  // Adjust this constant as needed

    // Map the distance to a grayscale value (white when close, black when far away)
    float intensity = 1.0 - clamp(distanceFromCamera / maxDistance, 0.0, 1.0);

    // Set the fragment color to a grayscale value
    fragColor = vec3(intensity);
    fragUv = uv;
    // fragColor = vec3(texture(texSampler, uv));
}
