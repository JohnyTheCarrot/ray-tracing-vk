#ifndef SRC_VULKAN_LOGICAL_DEVICE_H_
#define SRC_VULKAN_LOGICAL_DEVICE_H_

#include "vkb_raii.h"
#include <cstdint>

namespace raytracing::vulkan {
	class PhysicalDevice;

	class LogicalDevice final {
		UniqueVkbDevice       device_;
		PhysicalDevice const *phys_device_;

	public:
		LogicalDevice(UniqueVkbDevice &&device, PhysicalDevice const &phys_device);

		[[nodiscard]]
		vkb::Device &get() noexcept;

		[[nodiscard]]
		vkb::Device const &get() const noexcept;

		[[nodiscard]]
		PhysicalDevice const &get_phys() const noexcept;

		VkQueue get_queue(std::uint32_t index) const noexcept;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_LOGICAL_DEVICE_H_
