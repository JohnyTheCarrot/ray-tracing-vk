#include "descriptor_set_layout.h"
#include "src/diagnostics.h"
#include "src/vulkan/vk_exception.h"
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	VkDescriptorSetLayoutDestroyer::VkDescriptorSetLayoutDestroyer(VkDevice device)
	    : device_{device} {
	}

	void VkDescriptorSetLayoutDestroyer::operator()(VkDescriptorSetLayout descriptor_set_layout) const {
		Logger::get_instance().log(LogLevel::Debug, "Destroying descriptor set layout");
		vkDestroyDescriptorSetLayout(device_, descriptor_set_layout, nullptr);
	}

	UniqueVkDescriptorSetLayout create_descriptor_set_layout(
	        VkDevice device, std::vector<VkDescriptorBindingFlags> binding_flags,
	        std::span<VkDescriptorSetLayoutBinding> bindings
	) {
		auto final_binding_flags{binding_flags};
		if (binding_flags.empty() || binding_flags.size() < bindings.size()) {
			final_binding_flags.resize(bindings.size(), 0);
		}

		VkDescriptorSetLayoutBindingFlagsCreateInfo bindings_info{
		        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO
		};
		bindings_info.bindingCount  = static_cast<std::uint32_t>(bindings.size());
		bindings_info.pBindingFlags = final_binding_flags.data();

		VkDescriptorSetLayoutCreateInfo create_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
		create_info.bindingCount = bindings_info.bindingCount;
		create_info.pBindings    = bindings.data();
		create_info.flags        = 0;
		create_info.pNext        = final_binding_flags.empty() ? nullptr : &bindings_info;

		VkDescriptorSetLayout desc_set_layout{};
		if (VkResult const result{vkCreateDescriptorSetLayout(device, &create_info, nullptr, &desc_set_layout)};
		    result != VK_SUCCESS) {
			throw VkException{"Failed to create descriptor set layout", result};
		}

		return UniqueVkDescriptorSetLayout{desc_set_layout, VkDescriptorSetLayoutDestroyer{device}};
	}
}// namespace raytracing::vulkan
