#ifndef SRC_MODEL_H_
#define SRC_MODEL_H_

#include "src/mesh.h"
#include "src/vulkan/acc_struct.h"
#include <cstdint>
#include <filesystem>
#include <optional>
#include <unordered_map>

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

	struct MeshInstance final {
		glm::mat4     model_matrix_{};
		std::uint32_t mesh_idx_{};
	};

	struct SceneNode final {
		glm::mat4                    local_matrix_{};
		std::optional<std::uint32_t> mesh_idx_{};
		SceneNode                   *parent_;
	};

	class Scene final {
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

		std::vector<Mesh>                            meshes_;
		std::vector<SceneNode>                       nodes_;
		std::vector<BuildAccelerationStructure>      blas_{};
		std::optional<vulkan::AccelerationStructure> tlas_{};

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

		[[nodiscard]]
		vulkan::AccelerationStructure create_tlas(
		        vulkan::LogicalDevice const &device, VmaAllocator allocator,
		        vulkan::CommandBuffer const &command_buffer, std::vector<BuildAccelerationStructure> const &blas,
		        std::vector<MeshInstance> const &mesh_instances
		);

	public:
		Scene(vulkan::LogicalDevice const &device, vulkan::CommandPool const &command_pool, VmaAllocator allocator,
		      std::filesystem::path const &path, GltfScene);
	};
}// namespace raytracing

#endif//  SRC_MODEL_H_
