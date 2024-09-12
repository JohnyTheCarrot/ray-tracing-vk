#ifndef SRC_VULKAN_DESCRIPTOR_SET_LAYOUT_H_
#define SRC_VULKAN_DESCRIPTOR_SET_LAYOUT_H_

#include <memory>
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	class VkDescriptorSetLayoutDestroyer final {
		VkDevice device_;

	public:
		explicit VkDescriptorSetLayoutDestroyer(VkDevice device);

		void operator()(VkDescriptorSetLayout descriptor_set_layout) const;
	};

	using UniqueVkDescriptorSetLayout = std::unique_ptr<VkDescriptorSetLayout_T, VkDescriptorSetLayoutDestroyer>;
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_DESCRIPTOR_SET_LAYOUT_H_
