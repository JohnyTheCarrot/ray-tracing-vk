#ifndef SRC_VULKAN_PHYS_DEVICE_H_
#define SRC_VULKAN_PHYS_DEVICE_H_

#include "VkBootstrap.h"
#include "src/vulkan/logical_device.h"

namespace raytracing::vulkan {
	class PhysicalDevice final {
		vkb::PhysicalDevice device_;

	public:
		explicit PhysicalDevice(vkb::PhysicalDevice &&device);

		[[nodiscard]]
		vkb::PhysicalDevice &get() noexcept;

		[[nodiscard]]
		vkb::PhysicalDevice const &get() const noexcept;

		LogicalDevice create_logical_device() const;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_PHYS_DEVICE_H_
