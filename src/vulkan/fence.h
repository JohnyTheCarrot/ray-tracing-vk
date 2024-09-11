#ifndef SRC_VULKAN_FENCE_H_
#define SRC_VULKAN_FENCE_H_

#include <memory>
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	class VkFenceDestroyer final {
		VkDevice device_;

	public:
		explicit VkFenceDestroyer(VkDevice device);

		void operator()(VkFence fence) const;
	};

	using UniqueVkFence = std::unique_ptr<VkFence_T, VkFenceDestroyer>;
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_FENCE_H_
