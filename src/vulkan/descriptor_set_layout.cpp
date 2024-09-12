#include "descriptor_set_layout.h"
#include "src/diagnostics.h"
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	VkDescriptorSetLayoutDestroyer::VkDescriptorSetLayoutDestroyer(VkDevice device)
	    : device_{device} {
	}

	void VkDescriptorSetLayoutDestroyer::operator()(VkDescriptorSetLayout descriptor_set_layout) const {
		Logger::get_instance().log(LogLevel::Debug, "Destroying descriptor set layout");
		vkDestroyDescriptorSetLayout(device_, descriptor_set_layout, nullptr);
	}
}// namespace raytracing::vulkan
