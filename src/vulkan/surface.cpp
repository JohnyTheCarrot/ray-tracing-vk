#include "surface.h"
#include "src/diagnostics.h"
#include "src/window.h"
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	VkSurfaceDestroyer::VkSurfaceDestroyer(VkInstance instance)
	    : instance_{instance} {
	}

	void VkSurfaceDestroyer::operator()(VkSurfaceKHR surface) const {
		Logger::get_instance().log(LogLevel::Debug, "Destroying surface");
		vkDestroySurfaceKHR(instance_, surface, nullptr);
	}

	Surface::Surface(VkInstance instance, VkSurfaceKHR surface)
	    : surface_{surface, VkSurfaceDestroyer{instance}} {
	}
}// namespace raytracing::vulkan
