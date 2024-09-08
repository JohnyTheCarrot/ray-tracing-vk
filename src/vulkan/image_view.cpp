#include "image_view.h"
#include "src/diagnostics.h"
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	ImageViewDestroyer::ImageViewDestroyer(VkDevice device)
	    : device_{device} {
	}

	void ImageViewDestroyer::operator()(VkImageView imageView) {
		Logger::get_instance().log(LogLevel::Debug, "Destroying image view");
		vkDestroyImageView(device_, imageView, nullptr);
	}
}// namespace raytracing::vulkan
