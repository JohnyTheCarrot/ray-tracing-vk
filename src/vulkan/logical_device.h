#ifndef SRC_VULKAN_LOGICAL_DEVICE_H_
#define SRC_VULKAN_LOGICAL_DEVICE_H_

#include "VkBootstrap.h"
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

		[[nodiscard]]
		VkQueue get_queue(std::uint32_t index) const noexcept;

		[[nodiscard]]
		VkQueue get_queue(vkb::QueueType queue_type) const;

		[[nodiscard]]
		std::uint32_t get_queue_index(vkb::QueueType queue_type) const;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_LOGICAL_DEVICE_H_
