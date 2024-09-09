#include "shader_module.h"
#include "src/vulkan/vk_exception.h"
#include "src/vulkan/vkb_raii.h"
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	ShaderModule::ShaderModule(VkDevice device, std::span<char> code)
	    : shader_module_{[&] {
		    VkShaderModuleCreateInfo create_info{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
		    create_info.codeSize = code.size_bytes();
		    create_info.pCode    = reinterpret_cast<std::uint32_t const *>(code.data());

		    VkShaderModule shader_module{};
		    if (VkResult const result{vkCreateShaderModule(device, &create_info, nullptr, &shader_module)};
		        result != VK_SUCCESS) {
			    throw VkException{"Failed to create shader module", result};
		    }

		    return UniqueVkShaderModule{shader_module, VkShaderModuleDestroyer{device}};
	    }()} {
	}

	VkShaderModule ShaderModule::get() const {
		return shader_module_.get();
	}

	VkPipelineShaderStageCreateInfo ShaderModule::create_stage_create_info(VkShaderStageFlagBits stage) const {
		VkPipelineShaderStageCreateInfo create_info{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
		create_info.stage  = stage;
		create_info.module = shader_module_.get();
		create_info.pName  = "main";

		return create_info;
	}
}// namespace raytracing::vulkan
