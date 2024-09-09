#include "phys_device.h"
#include "VkBootstrap.h"
#include "src/diagnostics.h"
#include "src/vulkan/logical_device.h"
#include <format>
#include <stdexcept>

namespace raytracing::vulkan {
	PhysicalDevice::PhysicalDevice(vkb::PhysicalDevice &&device)
	    : phys_device_{std::move(device)} {
	}

	vkb::PhysicalDevice &PhysicalDevice::get() noexcept {
		return phys_device_;
	}

	vkb::PhysicalDevice const &PhysicalDevice::get() const noexcept {
		return phys_device_;
	}

	LogicalDevice PhysicalDevice::create_logical_device() const {
		Logger::get_instance().log(LogLevel::Debug, "Creating logical device");
		vkb::DeviceBuilder const device_builder{phys_device_};
		auto                     dev_ret{device_builder.build()};

		if (!dev_ret) {
			std::string message{std::format("Could not create logical device: {}", dev_ret.error().message())};
			throw std::runtime_error{std::move(message)};
		}

		return LogicalDevice{std::move(dev_ret.value()), *this};
	}

	VkPhysicalDeviceProperties2 PhysicalDevice::get_properties() const {
		VkPhysicalDeviceProperties2 device_props = {};
		device_props.sType                       = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

		vkGetPhysicalDeviceProperties2(phys_device_.physical_device, &device_props);

		return device_props;
	}

	VkPhysicalDeviceAccelerationStructurePropertiesKHR PhysicalDevice::get_as_properties() const {
		VkPhysicalDeviceAccelerationStructurePropertiesKHR acc_props{};
		acc_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;

		VkPhysicalDeviceProperties2 device_props{};
		device_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		device_props.pNext = &acc_props;

		vkGetPhysicalDeviceProperties2(phys_device_.physical_device, &device_props);

		return acc_props;
	}
}// namespace raytracing::vulkan
