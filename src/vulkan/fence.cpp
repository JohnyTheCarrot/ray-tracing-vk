#include "fence.h"
#include "src/diagnostics.h"
#include "vk_exception.h"
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	VkFenceDestroyer::VkFenceDestroyer(VkDevice device)
	    : device_{device} {
	}

	void VkFenceDestroyer::operator()(VkFence fence) const {
		Logger::get_instance().log(LogLevel::Debug, "Destroying fence");
		vkDestroyFence(device_, fence, nullptr);
	}

	UniqueVkFence create_fence(VkDevice device, VkFenceCreateFlags flags) {
		VkFenceCreateInfo fence_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
		fence_info.flags = flags;

		VkFence fence{};
		if (VkResult const result{vkCreateFence(device, &fence_info, nullptr, &fence)}; result != VK_SUCCESS) {
			throw VkException{"Could not create fence", result};
		}

		return vulkan::UniqueVkFence{fence, vulkan::VkFenceDestroyer{device}};
	}

}// namespace raytracing::vulkan
