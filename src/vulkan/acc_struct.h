#ifndef SRC_VULKAN_ACC_STRUCT_H_
#define SRC_VULKAN_ACC_STRUCT_H_

#include "src/vulkan/buffer.h"
#include <memory>

struct VkDevice_T;
using VkDevice = VkDevice_T *;

struct VkPhysicalDevice_T;
using VkPhysicalDevice = VkPhysicalDevice_T *;

struct VkAccelerationStructureKHR_T;
using VkAccelerationStructureKHR = VkAccelerationStructureKHR_T *;

namespace raytracing::vulkan {
	class AccelerationStructureDestroyer final {
		VkDevice device_;

	public:
		explicit AccelerationStructureDestroyer(VkDevice device);

		void operator()(VkAccelerationStructureKHR acc) const;
	};

	using UniqueVkAccelerationStructure = std::unique_ptr<VkAccelerationStructureKHR_T, AccelerationStructureDestroyer>;

	class AccelerationStructure final {
		Buffer                        buffer_;
		UniqueVkAccelerationStructure acc_;

	public:
		AccelerationStructure(
		        VkDevice device, VmaAllocator allocator, VkAccelerationStructureCreateInfoKHR const &create_info
		);

		[[nodiscard]]
		VkAccelerationStructureKHR get_acc() const;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_ACC_STRUCT_H_
