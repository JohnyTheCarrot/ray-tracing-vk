#ifndef SRC_VULKAN_EXT_FNS_H_
#define SRC_VULKAN_EXT_FNS_H_

#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan::ext {
	void vkGetAccelerationStructureBuildSizesKHR(
	        VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
	        VkAccelerationStructureBuildGeometryInfoKHR const *pBuildInfo, uint32_t const *pMaxPrimitiveCounts,
	        VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo
	);

	[[nodiscard]]
	VkResult vkCreateAccelerationStructureKHR(
	        VkDevice device, VkAccelerationStructureCreateInfoKHR const *pCreateInfo,
	        VkAllocationCallbacks const *pAllocator, VkAccelerationStructureKHR *pAccelerationStructure
	);

	void vkDestroyAccelerationStructureKHR(
	        VkDevice device, VkAccelerationStructureKHR accelerationStructure, VkAllocationCallbacks const *pAllocator
	);

	[[nodiscard]]
	VkDeviceAddress vkGetAccelerationStructureDeviceAddressKHR(
	        VkDevice device, VkAccelerationStructureDeviceAddressInfoKHR const *addressInfo
	);

	void vkCmdBuildAccelerationStructuresKHR(
	        VkDevice device, VkCommandBuffer commandBuffer, uint32_t infoCount,
	        VkAccelerationStructureBuildGeometryInfoKHR const     *pInfos,
	        VkAccelerationStructureBuildRangeInfoKHR const *const *ppBuildRangeInfos
	);
}// namespace raytracing::vulkan::ext

#endif//  SRC_VULKAN_EXT_FNS_H_
