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

	VkDeviceAddress vkGetAccelerationStructureDeviceAddressKHR(
	        VkDevice device, VkAccelerationStructureDeviceAddressInfoKHR const *addressInfo
	) {
		auto const func{reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(
		        vkGetDeviceProcAddr(device, "vkGetAccelerationStructureDeviceAddressKHR")
		)};

		if (func == nullptr) {
			throw std::runtime_error{"Failed to get vkGetAccelerationStructureDeviceAddressKHR function"};
		}

		return func(device, addressInfo);
	}

	void vkCmdBuildAccelerationStructuresKHR(
	        VkDevice device, VkCommandBuffer commandBuffer, uint32_t infoCount,
	        VkAccelerationStructureBuildGeometryInfoKHR const     *pInfos,
	        VkAccelerationStructureBuildRangeInfoKHR const *const *ppBuildRangeInfos
	) {
		auto const func{reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(
		        vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR")
		)};

		if (func == nullptr) {
			throw std::runtime_error{"Failed to get vkCmdBuildAccelerationStructuresKHR function"};
		}

		return func(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
	}
}// namespace raytracing::vulkan::ext
