#ifndef SRC_MESH_H_
#define SRC_MESH_H_

#include "src/vulkan/buffer.h"
#include "src/vulkan/host_device.h"
#include "src/vulkan/vkb_raii.h"
#include <cstdint>
#include <vector>

namespace raytracing {
	namespace vulkan {
		class CommandBuffer;
	}

	using MeshIndex = std::uint32_t;

	class Mesh final {
		std::vector<MeshIndex> indices_;
		std::vector<Vertex>    vertices_;
		vulkan::Buffer         index_buffer_;
		vulkan::Buffer         vertex_buffer_;

	public:
		Mesh(VmaAllocator allocator, vulkan::CommandBuffer const &command_buffer, std::vector<MeshIndex> &&indices,
		     std::vector<Vertex> &&vertices);
	};
}// namespace raytracing

#endif//  SRC_MESH_H_
