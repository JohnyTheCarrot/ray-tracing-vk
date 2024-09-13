#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUv;
layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    //outColor = vec4(fragUv, 1.0, 1.0);
    outColor = texture(texSampler, fragUv);
}
