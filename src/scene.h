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

namespace raytracing::vulkan {
	struct GltfScene final {};

	class PhysicalDevice;

	class LogicalDevice;

	class CommandPool;

	class CommandBuffer;

	struct MeshInstance final {
		glm::mat4     model_matrix_{};
		std::uint32_t mesh_idx_{};
	};

	struct SceneNode final {
		glm::mat4                    local_matrix_{};
		std::optional<std::uint32_t> mesh_idx_{};
		SceneNode                   *parent_;
	};

	enum class SceneFormat { Gltf };

	class Scene final {
		struct BuildAccelerationStructure final {
			VkAccelerationStructureBuildGeometryInfoKHR build_info_{
			        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR
			};
			VkAccelerationStructureBuildSizesInfoKHR size_info_{
			        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR
			};
			VkAccelerationStructureBuildRangeInfoKHR const *range_info_{};
			std::optional<AccelerationStructure>            acc_;
		};

		std::vector<Mesh>                       meshes_;
		std::vector<SceneNode>                  nodes_;
		std::vector<BuildAccelerationStructure> blas_{};
		std::optional<AccelerationStructure>    tlas_{};

		void cmd_create_blas(
		        CommandBuffer const &command_buffer, VkDevice device, VkPhysicalDevice phys_device,
		        VmaAllocator allocator, std::vector<std::uint32_t> const &indices,
		        std::vector<BuildAccelerationStructure> &build_structures, VkDeviceAddress scratch_address
		) const;

		[[nodiscard]]
		std::vector<BuildAccelerationStructure> create_blas(
		        VkPhysicalDevice phys_device, CommandPool const &command_pool, VmaAllocator allocator, VkDevice device
		);

		[[nodiscard]]
		AccelerationStructure create_tlas(
		        LogicalDevice const &device, VmaAllocator allocator, CommandPool const &command_pool,
		        std::vector<BuildAccelerationStructure> const               &blas,
		        std::unordered_map<MeshIndex, std::vector<glm::mat4>> const &mesh_instances
		);

	public:
		Scene(LogicalDevice const &device, CommandPool const &command_pool, VmaAllocator allocator,
		      std::filesystem::path const &path, GltfScene);

		void rasterizer_draw(VkCommandBuffer render_buffer, VkPipelineLayout pipeline_layout, VkDescriptorSet desc_set)
		        const;
	};
}// namespace raytracing::vulkan

#endif//  SRC_MODEL_H_
