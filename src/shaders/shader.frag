#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUv;
layout(binding = 1) uniform sampler2D texSamplers[];
layout(push_constant, std430) uniform PushConstant {
    uint tex_idx;
} pc;

layout(location = 0) out vec4 outColor;

void main() {
    //outColor = vec4(fragUv, 1.0, 1.0);
    outColor = texture(nonuniformEXT(texSamplers[pc.tex_idx]), fragUv);
}
