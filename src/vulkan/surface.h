#ifndef SRC_VULKAN_SURFACE_H_
#define SRC_VULKAN_SURFACE_H_

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

	public:
		Surface(VkInstance instance, VkSurfaceKHR surface);
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_SURFACE_H_
