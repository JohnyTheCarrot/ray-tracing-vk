#include "phys_device.h"
#include "VkBootstrap.h"
#include "src/diagnostics.h"
#include "src/vulkan/logical_device.h"
#include <format>
#include <stdexcept>

namespace raytracing::vulkan {
	PhysicalDevice::PhysicalDevice(vkb::PhysicalDevice &&device)
	    : device_{std::move(device)} {
	}

	vkb::PhysicalDevice &PhysicalDevice::get() noexcept {
		return device_;
	}

	vkb::PhysicalDevice const &PhysicalDevice::get() const noexcept {
		return device_;
	}

	LogicalDevice PhysicalDevice::create_logical_device() const {
		Logger::get_instance().log(LogLevel::Debug, "Creating logical device");
		vkb::DeviceBuilder const device_builder{device_};
		auto                     dev_ret{device_builder.build()};

		if (!dev_ret) {
			std::string message{std::format("Could not create logical device: {}", dev_ret.error().message())};
			throw std::runtime_error{std::move(message)};
		}

		return LogicalDevice{std::move(dev_ret.value())};
	}
}// namespace raytracing::vulkan
