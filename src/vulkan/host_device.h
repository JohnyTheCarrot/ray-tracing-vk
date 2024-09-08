#ifndef SRC_VULKAN_HOST_DEVICE_H_
#define SRC_VULKAN_HOST_DEVICE_H_

#ifdef __cplusplus
#include <glm/glm.hpp>
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using uint = unsigned int;
#endif

// clang-format off
#ifdef __cplusplus
#define ENUM(name) enum name {
#define END_ENUM() }
#else
#define ENUM(name) const uint
#define END_ENUM()
#endif

// clang-format on

struct Vertex {
	vec3 pos;
	vec3 norm;
	vec2 uv;
};

#endif//  SRC_VULKAN_HOST_DEVICE_H_
