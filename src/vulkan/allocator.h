#ifndef SRC_VULKAN_ALLOCATOR_H_
#define SRC_VULKAN_ALLOCATOR_H_

#include "vkb_raii.h"

namespace raytracing::vulkan {
	class PhysicalDevice;

	class LogicalDevice;

	class Instance;

	// TODO: not much point for this class to exist over just a function that returns a newly made UniqueVmaAllocator
	class Allocator final {
		UniqueVmaAllocator allocator_;

	public:
		Allocator(VkInstance instance, PhysicalDevice const &physical_device, LogicalDevice const &logical_device);

		[[nodiscard]]
		VmaAllocator get() const noexcept;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_ALLOCATOR_H_
