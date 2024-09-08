#include "ext_fns.h"
#include <stdexcept>

namespace raytracing::vulkan::ext {
	void vkGetAccelerationStructureBuildSizesKHR(
	        VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
	        VkAccelerationStructureBuildGeometryInfoKHR const *pBuildInfo, uint32_t const *pMaxPrimitiveCounts,
	        VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo
	) {
		auto const func{reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(
		        vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR")
		)};

		if (func == nullptr) {
			throw std::runtime_error{"Failed to get vkGetAccelerationStructureBuildSizesKHR function"};
		}

		return func(device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
	}

	VkResult vkCreateAccelerationStructureKHR(
	        VkDevice device, VkAccelerationStructureCreateInfoKHR const *pCreateInfo,
	        VkAllocationCallbacks const *pAllocator, VkAccelerationStructureKHR *pAccelerationStructure
	) {
		auto const func{reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(
		        vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR")
		)};

		if (func == nullptr) {
			throw std::runtime_error{"Failed to get vkCreateAccelerationStructureKHR function"};
		}

		return func(device, pCreateInfo, pAllocator, pAccelerationStructure);
	}

	void vkDestroyAccelerationStructureKHR(
	        VkDevice device, VkAccelerationStructureKHR accelerationStructure, VkAllocationCallbacks const *pAllocator
	) {
		auto const func{reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(
		        vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR")
		)};

		if (func == nullptr) {
			throw std::runtime_error{"Failed to get vkDestroyAccelerationStructureKHR function"};
		}

		return func(device, accelerationStructure, pAllocator);
	}
}// namespace raytracing::vulkan::ext
