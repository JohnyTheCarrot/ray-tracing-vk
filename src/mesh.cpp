#include "mesh.h"

#include "src/vulkan/vkb_raii.h"
#include <vulkan/vulkan_core.h>

namespace raytracing {
	constexpr VkBufferUsageFlags index_buffer_usage_flags{
	        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
	        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
	        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
	};
	constexpr VkBufferUsageFlags vertex_buffer_usage_flags{
	        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
	        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
	        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
	};

	Mesh::Mesh(
	        VmaAllocator allocator, vulkan::CommandBuffer const &command_buffer, std::vector<std::uint32_t> &&indices,
	        std::vector<Vertex> &&vertices
	)
	    : indices_{std::move(indices)}
	    , vertices_{std::move(vertices)}
	    , index_buffer_{[&] {
		    std::span<MeshIndex> span{indices_};
		    vulkan::Buffer       staging_buffer{allocator, span, VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
		    vulkan::Buffer       device_local_buffer{allocator, span.size_bytes(), index_buffer_usage_flags, 0};

		    staging_buffer.copy_to(command_buffer, device_local_buffer);

		    return device_local_buffer;
	    }()}
	    , vertex_buffer_{[&] {
		    std::span<Vertex> span{vertices_};
		    vulkan::Buffer    staging_buffer{allocator, span, VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
		    vulkan::Buffer    device_local_buffer{allocator, span.size_bytes(), vertex_buffer_usage_flags, 0};

		    staging_buffer.copy_to(command_buffer, device_local_buffer);

		    return device_local_buffer;
	    }()} {
	}
}// namespace raytracing
