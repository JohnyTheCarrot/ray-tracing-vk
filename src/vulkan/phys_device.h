#ifndef SRC_VULKAN_PHYS_DEVICE_H_
#define SRC_VULKAN_PHYS_DEVICE_H_

#include "VkBootstrap.h"
#include "src/vulkan/logical_device.h"

namespace raytracing::vulkan {
	class PhysicalDevice final {
		vkb::PhysicalDevice phys_device_;

	public:
		explicit PhysicalDevice(vkb::PhysicalDevice &&device);

		[[nodiscard]]
		vkb::PhysicalDevice &get() noexcept;

		[[nodiscard]]
		vkb::PhysicalDevice const &get() const noexcept;

		[[nodiscard]]
		LogicalDevice create_logical_device() const;

		[[nodiscard]]
		VkPhysicalDeviceProperties2 get_properties() const;

		[[nodiscard]]
		VkPhysicalDeviceAccelerationStructurePropertiesKHR get_as_properties() const;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_PHYS_DEVICE_H_
