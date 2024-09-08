#ifndef SRC_MESH_H_
#define SRC_MESH_H_

#include "src/vulkan/buffer.h"
#include "src/vulkan/host_device.h"
#include "src/vulkan/vkb_raii.h"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace raytracing {
	namespace vulkan {
		class CommandBuffer;
	}

	using MeshIndex = std::uint32_t;

	struct MeshBlasInput final {
		std::vector<VkAccelerationStructureGeometryKHR>       acc_structure_geom;
		std::vector<VkAccelerationStructureBuildRangeInfoKHR> acc_structure_build_offset_info;
	};

	class Mesh final {
		vulkan::Buffer index_buffer_;
		vulkan::Buffer vertex_buffer_;

	public:
		Mesh(VkDevice device, VmaAllocator allocator, vulkan::CommandBuffer const &command_buffer,
		     std::vector<MeshIndex> &indices, std::vector<Vertex> &vertices);

		[[nodiscard]]
		MeshBlasInput to_blas_input() const;
	};
}// namespace raytracing

#endif//  SRC_MESH_H_
