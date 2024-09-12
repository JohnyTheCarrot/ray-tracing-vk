#ifndef SRC_VULKAN_LOGICAL_DEVICE_H_
#define SRC_VULKAN_LOGICAL_DEVICE_H_

#include "VkBootstrap.h"
#include "src/vulkan/descriptor_set_layout.h"
#include "src/vulkan/fence.h"
#include "src/vulkan/image.h"
#include "src/vulkan/semaphore.h"
#include "vkb_raii.h"
#include <cstdint>
#include <span>
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	class PhysicalDevice;

	class Allocator;

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

		void wait_idle() const;

		[[nodiscard]]
		UniqueVkSemaphore create_semaphore() const;

		[[nodiscard]]
		UniqueVkFence create_fence(VkFenceCreateFlags flags = 0) const;

		[[nodiscard]]
		UniqueVkDescriptorSetLayout create_descriptor_set_layout(
		        std::vector<VkDescriptorBindingFlags> const  &binding_flags,
		        std::span<VkDescriptorSetLayoutBinding const> bindings
		) const;

		[[nodiscard]]
		Image create_image(
		        Allocator const &allocator, std::uint32_t width, std::uint32_t height, VkFormat format,
		        VkImageUsageFlags usage_flags
		) const;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_LOGICAL_DEVICE_H_
