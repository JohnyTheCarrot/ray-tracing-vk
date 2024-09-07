#ifndef SRC_VULKAN_SWAPCHAIN_H_
#define SRC_VULKAN_SWAPCHAIN_H_

#include "src/vulkan/image_view.h"
#include "src/vulkan/vkb_raii.h"
#include <optional>

namespace raytracing::vulkan {
	class LogicalDevice;

	class Swapchain final {
		UniqueVkbSwapchain           swapchain_;
		std::vector<UniqueImageView> image_views_;
		LogicalDevice const         *device_;

	public:
		explicit Swapchain(LogicalDevice const &device, std::optional<Swapchain> &&old = std::nullopt);

		[[nodiscard]]
		vkb::Swapchain &get() noexcept;

		[[nodiscard]]
		vkb::Swapchain const &get() const noexcept;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_SWAPCHAIN_H_
