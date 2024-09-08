#ifndef SRC_MODEL_H_
#define SRC_MODEL_H_

#include "src/mesh.h"
#include "src/vulkan/acc_struct.h"
#include <cstdint>
#include <filesystem>
#include <optional>

struct VkDevice_T;
using VkDevice = VkDevice_T *;

struct VkPhysicalDevice_T;
using VkPhysicalDevice = VkPhysicalDevice_T *;

namespace raytracing {
	struct GltfScene final {};

	namespace vulkan {
		class PhysicalDevice;

		class LogicalDevice;

		class CommandPool;

		class CommandBuffer;
	}// namespace vulkan

	class Scene final {
		std::vector<Mesh> meshes_;

		struct BuildAccelerationStructure final {
			VkAccelerationStructureBuildGeometryInfoKHR build_info_{
			        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR
			};
			VkAccelerationStructureBuildSizesInfoKHR size_info_{
			        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR
			};
			VkAccelerationStructureBuildRangeInfoKHR const *range_info_{};
			std::optional<vulkan::AccelerationStructure>    acc_;
		};

		void cmd_create_blas(
		        VkDevice device, VkPhysicalDevice phys_device, VmaAllocator allocator,
		        std::vector<std::uint32_t> const &indices, std::vector<BuildAccelerationStructure> &build_structures,
		        VkDeviceAddress scratch_address
		) const;

		[[nodiscard]]
		std::vector<BuildAccelerationStructure> create_blas(
		        VkPhysicalDevice phys_device, vulkan::CommandBuffer const &command_buffer, VmaAllocator allocator,
		        VkDevice device
		) const;

	public:
		Scene(vulkan::LogicalDevice const &device, vulkan::CommandPool const &command_pool, VmaAllocator allocator,
		      std::filesystem::path const &path, GltfScene);
	};
}// namespace raytracing

#endif//  SRC_MODEL_H_
