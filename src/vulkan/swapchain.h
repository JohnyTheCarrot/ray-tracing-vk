#ifndef SRC_VULKAN_SWAPCHAIN_H_
#define SRC_VULKAN_SWAPCHAIN_H_

#include "src/vulkan/vkb_raii.h"

namespace raytracing::vulkan {
	class LogicalDevice;

	class Swapchain final {
		UniqueVkbSwapchain swapchain_;
		vkb::Device const *device_;

	public:
		explicit Swapchain(LogicalDevice const &device);

		void recreate();

		[[nodiscard]]
		vkb::Swapchain &get() noexcept;

		[[nodiscard]]
		vkb::Swapchain const &get() const noexcept;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_SWAPCHAIN_H_
