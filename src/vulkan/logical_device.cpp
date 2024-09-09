#include "logical_device.h"
#include "phys_device.h"
#include <format>

namespace raytracing::vulkan {
	LogicalDevice::LogicalDevice(UniqueVkbDevice &&device, PhysicalDevice const &phys_device)
	    : device_{std::move(device)}
	    , phys_device_{&phys_device} {
	}

	vkb::Device &LogicalDevice::get() noexcept {
		return device_.get();
	}

	vkb::Device const &LogicalDevice::get() const noexcept {
		return device_.get();
	}

	PhysicalDevice const &LogicalDevice::get_phys() const noexcept {
		return *phys_device_;
	}

	VkQueue LogicalDevice::get_queue(std::uint32_t index) const noexcept {
		VkQueue queue;
		vkGetDeviceQueue(device_.get().device, index, 0, &queue);

		return queue;
	}

	VkQueue LogicalDevice::get_queue(vkb::QueueType queue_type) const {
		auto const queue_idx{get().get_queue(queue_type)};
		if (!queue_idx) {
			std::string message{std::format("Failed to get queue: {}", queue_idx.error().message())};
			throw std::runtime_error{std::move(message)};
		}

		return queue_idx.value();
	}

	std::uint32_t LogicalDevice::get_queue_index(vkb::QueueType queue_type) const {
		auto const queue_idx{get().get_queue_index(queue_type)};
		if (!queue_idx) {
			std::string message{std::format("Failed to get queue index: {}", queue_idx.error().message())};
			throw std::runtime_error{std::move(message)};
		}

		return queue_idx.value();
	}
}// namespace raytracing::vulkan
