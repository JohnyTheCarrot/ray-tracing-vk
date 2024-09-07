#ifndef SRC_VULKAN_SURFACE_H_
#define SRC_VULKAN_SURFACE_H_

#include "VkBootstrap.h"
#include "src/vulkan/phys_device.h"
#include <memory>

struct VkSurfaceKHR_T;
using VkSurfaceKHR = VkSurfaceKHR_T *;

struct VkInstance_T;
using VkInstance = VkInstance_T *;

namespace raytracing {
	class Window;
}

namespace raytracing::vulkan {
	class VkSurfaceDestroyer final {
		VkInstance instance_;

	public:
		VkSurfaceDestroyer(VkInstance instance);

		void operator()(VkSurfaceKHR surface) const;
	};

	using UniqueVkSurface = std::unique_ptr<VkSurfaceKHR_T, VkSurfaceDestroyer>;

	class Surface final {
		UniqueVkSurface surface_;
		vkb::Instance  *instance_;

	public:
		Surface(vkb::Instance &instance, VkSurfaceKHR surface);

		[[nodiscard]]
		PhysicalDevice select_physical_device();
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_SURFACE_H_
