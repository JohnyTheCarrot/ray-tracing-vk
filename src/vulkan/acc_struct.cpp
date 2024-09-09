#include "acc_struct.h"
#include "src/vulkan/ext_fns.h"
#include "src/vulkan/vk_exception.h"
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	AccelerationStructureDestroyer::AccelerationStructureDestroyer(VkDevice device)
	    : device_{device} {
	}

	void AccelerationStructureDestroyer::operator()(VkAccelerationStructureKHR acc) const {
		/*Logger::get_instance().log(LogLevel::Debug, "Destroying acceleration structure");*/
		vulkan::ext::vkDestroyAccelerationStructureKHR(device_, acc, nullptr);
	}

	AccelerationStructure::AccelerationStructure(
	        VkDevice device, VmaAllocator allocator, VkAccelerationStructureCreateInfoKHR const &create_info
	)
	    : buffer_{device, allocator, create_info.size,
	              VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, 0}
	    , acc_{[&] {
		    VkAccelerationStructureCreateInfoKHR complete_create_info{create_info};

		    complete_create_info.buffer = buffer_.get();

		    VkAccelerationStructureKHR acc{};
		    if (VkResult const result{
		                ext::vkCreateAccelerationStructureKHR(device, &complete_create_info, nullptr, &acc)
		        };
		        result != VK_SUCCESS) {
			    throw VkException{"Failed to create acceleration structure", result};
		    }

		    return UniqueVkAccelerationStructure{acc, AccelerationStructureDestroyer{device}};
	    }()} {
	}

	VkAccelerationStructureKHR AccelerationStructure::get_acc() const {
		return acc_.get();
	}
}// namespace raytracing::vulkan
