#include "logical_device.h"
#include "phys_device.h"

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
}// namespace raytracing::vulkan
