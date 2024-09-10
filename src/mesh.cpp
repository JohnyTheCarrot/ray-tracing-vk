#include "mesh.h"

#include "src/vulkan/vkb_raii.h"
#include <cstdint>
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
	        VkDevice device, VmaAllocator allocator, vulkan::CommandPool const &command_pool,
	        std::vector<std::uint32_t> &indices, std::vector<Vertex> &vertices
	)
	    : index_buffer_{[&] {
		    std::span<MeshIndex> span{indices};
		    vulkan::Buffer       staging_buffer{device, allocator, span, VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
		    vulkan::Buffer       device_local_buffer{device, allocator, span.size_bytes(), index_buffer_usage_flags, 0};

		    staging_buffer.copy_to(command_pool, device_local_buffer);

		    return device_local_buffer;
	    }()}
	    , vertex_buffer_{[&] {
		    std::span<Vertex> span{vertices};
		    vulkan::Buffer    staging_buffer{device, allocator, span, VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
		    vulkan::Buffer    device_local_buffer{device, allocator, span.size_bytes(), vertex_buffer_usage_flags, 0};

		    staging_buffer.copy_to(command_pool, device_local_buffer);

		    return device_local_buffer;
	    }()} {
	}

	MeshBlasInput Mesh::to_blas_input() const {
		VkDeviceAddress const index_buff_address{index_buffer_.get_device_address()};
		VkDeviceAddress const vertex_buff_address{vertex_buffer_.get_device_address()};

		auto const max_primitive_count{index_buffer_.get_size() / sizeof(MeshIndex) / 3};

		VkAccelerationStructureGeometryTrianglesDataKHR triangles{
		        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR
		};
		triangles.vertexFormat             = VK_FORMAT_R32G32B32_SFLOAT;
		triangles.vertexData.deviceAddress = vertex_buff_address;
		triangles.vertexStride             = sizeof(Vertex);
		triangles.indexType                = VK_INDEX_TYPE_UINT32;
		triangles.indexData.deviceAddress  = index_buff_address;
		triangles.maxVertex                = vertex_buffer_.get_size() / sizeof(Vertex) - 1;

		VkAccelerationStructureGeometryKHR acc_str_geom{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
		acc_str_geom.geometryType       = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		acc_str_geom.flags              = VK_GEOMETRY_OPAQUE_BIT_KHR;
		acc_str_geom.geometry.triangles = triangles;

		VkAccelerationStructureBuildRangeInfoKHR offset{};
		offset.firstVertex     = 0;
		offset.primitiveCount  = static_cast<std::uint32_t>(max_primitive_count);
		offset.primitiveOffset = 0;
		offset.transformOffset = 0;

		MeshBlasInput input{};
		input.acc_structure_geom.emplace_back(acc_str_geom);
		input.acc_structure_build_offset_info.emplace_back(offset);

		return input;
	}
}// namespace raytracing
