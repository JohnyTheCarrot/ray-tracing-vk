#include "shader_module.h"
#include "src/vulkan/vk_exception.h"
#include "src/vulkan/vkb_raii.h"
#include <fstream>
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	namespace shader_module {
		std::vector<char> read_file(std::filesystem::path const &path) {
			std::ifstream file{path, std::ios::binary | std::ios::ate};

			if (!file) {
				throw std::runtime_error{std::string{"couldn't open file " + path.string()}};
			}

			size_t            filesize = (size_t) file.tellg();
			std::vector<char> buffer(filesize);
			file.seekg(0);
			file.read(buffer.data(), filesize);

			file.close();
			return buffer;
		}
	}// namespace shader_module

	ShaderModule::ShaderModule(VkDevice device, std::vector<char> const &code)
	    : shader_module_{[&] {
		    VkShaderModuleCreateInfo create_info{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
		    create_info.codeSize = code.size();
		    create_info.pCode    = reinterpret_cast<std::uint32_t const *>(code.data());

		    VkShaderModule shader_module{};
		    if (VkResult const result{vkCreateShaderModule(device, &create_info, nullptr, &shader_module)};
		        result != VK_SUCCESS) {
			    throw VkException{"Failed to create shader module", result};
		    }

		    return UniqueVkShaderModule{shader_module, VkShaderModuleDestroyer{device}};
	    }()} {
	}

	ShaderModule::ShaderModule(VkDevice device, std::filesystem::path const &path)
	    : ShaderModule{device, shader_module::read_file(path)} {
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
