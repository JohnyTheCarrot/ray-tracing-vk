#ifndef SRC_VULKAN_ALLOCATOR_H_
#define SRC_VULKAN_ALLOCATOR_H_

#include "vkb_raii.h"

namespace raytracing::vulkan {
	class PhysicalDevice;

	class LogicalDevice;

	class Instance;

	class Allocator final {
		UniqueVmaAllocator allocator_;

	public:
		Allocator(
		        vkb::Instance const &instance, PhysicalDevice const &physical_device,
		        LogicalDevice const &logical_device
		);

		[[nodiscard]]
		VmaAllocator get() const noexcept;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_ALLOCATOR_H_
