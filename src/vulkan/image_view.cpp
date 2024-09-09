#include "image_view.h"
#include "src/diagnostics.h"
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	VkImageViewDestroyer::VkImageViewDestroyer(VkDevice device)
	    : device_{device} {
	}

	void VkImageViewDestroyer::operator()(VkImageView imageView) const {
		Logger::get_instance().log(LogLevel::Debug, "Destroying image view");
		vkDestroyImageView(device_, imageView, nullptr);
	}
}// namespace raytracing::vulkan
