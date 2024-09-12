#include "pipeline_layout.h"
#include "src/vulkan/vk_exception.h"
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	PipelineLayout::PipelineLayout(VkDevice device, VkDescriptorSetLayout desc_set_layout)
	    : pipeline_layout_{[=] {
		    VkPipelineLayoutCreateInfo pipeline_layout_info{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

		    pipeline_layout_info.setLayoutCount = 1;
		    pipeline_layout_info.pSetLayouts    = &desc_set_layout;
		    VkPipelineLayout pipeline_layout{};
		    if (VkResult const result{vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout)};
		        result != VK_SUCCESS) {
			    throw VkException{"Couldn't create pipeline layout", result};
		    }

		    return UniqueVkPipelineLayout{pipeline_layout, VkPipelineLayoutDestroyer{device}};
	    }()} {
	}

	VkPipelineLayout PipelineLayout::get() const {
		return pipeline_layout_.get();
	}
}// namespace raytracing::vulkan
