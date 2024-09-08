#include "allocator.h"
#include "logical_device.h"
#include "phys_device.h"
#include "src/vulkan/vk_exception.h"
#include "src/vulkan/vkb_raii.h"
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	Allocator::Allocator(
	        vkb::Instance const &instance, PhysicalDevice const &physical_device, LogicalDevice const &logical_device
	)
	    : allocator_{[&] {
		    VmaVulkanFunctions vulkan_functions{};

		    VmaAllocatorCreateInfo allocator_create_info{};
		    allocator_create_info.vulkanApiVersion = VK_API_VERSION_1_2;
		    allocator_create_info.physicalDevice   = physical_device.get().physical_device;
		    allocator_create_info.device           = logical_device.get().device;
		    allocator_create_info.instance         = instance.instance;
		    allocator_create_info.flags            = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

		    VmaAllocator allocator{};
		    if (VkResult const result{vmaCreateAllocator(&allocator_create_info, &allocator)}; result != VK_SUCCESS) {
			    throw VkException{"Failed to create VmaAllocator", result};
		    }

		    return UniqueVmaAllocator{allocator};
	    }()} {
	}

	VmaAllocator Allocator::get() const noexcept {
		return allocator_.get();
	}
}// namespace raytracing::vulkan
