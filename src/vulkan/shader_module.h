#ifndef SRC_VULKAN_SHADER_MODULE_H_
#define SRC_VULKAN_SHADER_MODULE_H_

#include "src/vulkan/vkb_raii.h"
#include <filesystem>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	class ShaderModule final {
		UniqueVkShaderModule shader_module_;

	public:
		ShaderModule(VkDevice device, std::filesystem::path const &path);

		ShaderModule(VkDevice device, std::vector<char> const &code);

		[[nodiscard]]
		VkShaderModule get() const;

		[[nodiscard]]
		VkPipelineShaderStageCreateInfo create_stage_create_info(VkShaderStageFlagBits stage) const;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_SHADER_MODULE_H_
