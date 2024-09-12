#include "mesh.h"

#include "src/diagnostics.h"
#include "src/vulkan/vkb_raii.h"
#include <cstdint>
#include <format>
#include <glm/fwd.hpp>
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

	constexpr VkBufferUsageFlags instance_buffer_usage_flags{
	        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
	};

	Mesh::Mesh(
	        VkDevice device, VmaAllocator allocator, vulkan::CommandPool const &command_pool,
	        std::vector<std::uint32_t> const &indices, std::vector<Vertex> const &vertices
	)
	    : index_buffer_{[&] {
		    std::span<MeshIndex const> span{indices};
		    vulkan::Buffer             staging_buffer{device, allocator, span, VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
		    vulkan::Buffer device_local_buffer{device, allocator, span.size_bytes(), index_buffer_usage_flags, 0};

		    staging_buffer.copy_to(command_pool, device_local_buffer);

		    return device_local_buffer;
	    }()}
	    , vertex_buffer_{[&] {
		    std::span<Vertex const> span{vertices};
		    vulkan::Buffer          staging_buffer{device, allocator, span, VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
		    vulkan::Buffer device_local_buffer{device, allocator, span.size_bytes(), vertex_buffer_usage_flags, 0};

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

	void Mesh::set_instances(
	        VkDevice device, VmaAllocator allocator, vulkan::CommandPool const &command_pool,
	        std::vector<glm::mat4> const &instances
	) {
		std::span<glm::mat4 const> span{instances};
		vulkan::Buffer             staging_buffer{device, allocator, span, VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
		instance_buffer_ = {device, allocator, span.size_bytes(), instance_buffer_usage_flags, 0};

		staging_buffer.copy_to(command_pool, instance_buffer_.value());
		Logger::get_instance().log(LogLevel::Debug, std::format("Setting {} instances", span.size()));
	}

	void Mesh::rasterizer_draw(
	        VkCommandBuffer render_buffer, VkPipelineLayout pipeline_layout, VkDescriptorSet desc_set
	) const {
		VkBuffer     vert_buff{vertex_buffer_.get()};
		VkBuffer     instance_buff{instance_buffer_->get()};
		VkDeviceSize offsets[]{0};
		vkCmdBindVertexBuffers(render_buffer, 0, 1, &vert_buff, offsets);
		vkCmdBindVertexBuffers(render_buffer, 1, 1, &instance_buff, offsets);
		vkCmdBindIndexBuffer(render_buffer, index_buffer_.get(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(
		        render_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &desc_set, 0, nullptr
		);
		std::uint32_t num_instances{static_cast<std::uint32_t>(instance_buffer_->get_size() / sizeof(glm::mat4))};
		std::uint32_t num_verts{static_cast<std::uint32_t>(index_buffer_.get_size() / sizeof(MeshIndex))};
		vkCmdDrawIndexed(render_buffer, num_verts, num_instances, 0, 0, 0);
	}
}// namespace raytracing
