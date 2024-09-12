#ifndef SRC_MESH_H_
#define SRC_MESH_H_

#include "src/vulkan/buffer.h"
#include "src/vulkan/host_device.h"
#include "src/vulkan/vkb_raii.h"
#include <cstdint>
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace raytracing {
	namespace vulkan {
		class CommandPool;
	}

	using MeshIndex = std::uint32_t;

	struct MeshBlasInput final {
		std::vector<VkAccelerationStructureGeometryKHR>       acc_structure_geom;
		std::vector<VkAccelerationStructureBuildRangeInfoKHR> acc_structure_build_offset_info;
	};

	class Mesh final {
		vulkan::Buffer                index_buffer_;
		vulkan::Buffer                vertex_buffer_;
		std::optional<vulkan::Buffer> instance_buffer_;

	public:
		Mesh(VkDevice device, VmaAllocator allocator, vulkan::CommandPool const &command_pool,
		     std::vector<MeshIndex> const &indices, std::vector<Vertex> const &vertices);

		void set_instances(
		        VkDevice device, VmaAllocator allocator, vulkan::CommandPool const &command_pool,
		        std::vector<glm::mat4> const &instances
		);

		[[nodiscard]]
		MeshBlasInput to_blas_input() const;

		void rasterizer_draw(VkCommandBuffer render_buffer, VkPipelineLayout pipeline_layout, VkDescriptorSet desc_set)
		        const;
	};
}// namespace raytracing

#endif//  SRC_MESH_H_
