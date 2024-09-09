#include "frame_buffer.h"
#include "src/diagnostics.h"
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	VkFramebufferDestroyer::VkFramebufferDestroyer(VkDevice device)
	    : device_{device} {
	}

	void VkFramebufferDestroyer::operator()(VkFramebuffer framebuffer) const {
		Logger::get_instance().log(LogLevel::Debug, "Destroying frame buffer");
		vkDestroyFramebuffer(device_, framebuffer, nullptr);
	}
}// namespace raytracing::vulkan
