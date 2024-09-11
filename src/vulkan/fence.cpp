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
}// namespace raytracing::vulkan
