#include "device_manager.h"
#include "VkBootstrap.h"
#include "src/vulkan/command_pool.h"
#include "src/vulkan/logical_device.h"
#include "src/vulkan/phys_device.h"

namespace raytracing::vulkan {
	DeviceManager::DeviceManager(VkInstance instance, PhysicalDevice &&device)
	    : physical_device_{std::move(device)}
	    , logical_device_{physical_device_.create_logical_device()}
	    , command_pool_{vkb::QueueType::graphics, logical_device_}
	    , allocator_{instance, physical_device_, logical_device_} {
	}

	LogicalDevice const &DeviceManager::get_logical() const {
		return logical_device_;
	}

	CommandPool const &DeviceManager::get_command_pool() const {
		return command_pool_;
	}

	Allocator const &DeviceManager::get_allocator() const {
		return allocator_;
	}
}// namespace raytracing::vulkan
