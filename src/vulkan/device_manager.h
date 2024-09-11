#ifndef SRC_VULKAN_DEVICE_MANAGER_H_
#define SRC_VULKAN_DEVICE_MANAGER_H_

#include "src/vulkan/allocator.h"
#include "src/vulkan/command_pool.h"
#include "src/vulkan/logical_device.h"
#include "src/vulkan/phys_device.h"

namespace raytracing::vulkan {
	class DeviceManager final {
		PhysicalDevice physical_device_;
		LogicalDevice  logical_device_;
		CommandPool    command_pool_;
		Allocator      allocator_;

	public:
		DeviceManager(VkInstance instance, PhysicalDevice &&device);

		[[nodiscard]]
		LogicalDevice const &get_logical() const;

		[[nodiscard]]
		CommandPool const &get_command_pool() const;

		[[nodiscard]]
		Allocator const &get_allocator() const;
	};
};// namespace raytracing::vulkan

#endif//  SRC_VULKAN_DEVICE_MANAGER_H_
