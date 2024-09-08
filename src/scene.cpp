#include "scene.h"
#include "src/diagnostics.h"
#include "src/vulkan/acc_struct.h"
#include "src/vulkan/buffer.h"
#include "src/vulkan/command_buffer.h"
#include "src/vulkan/command_pool.h"
#include "src/vulkan/ext_fns.h"
#include "src/vulkan/logical_device.h"
#include "src/vulkan/phys_device.h"
#include "src/vulkan/vk_exception.h"
#include "src/vulkan/vkb_raii.h"
#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <format>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace raytracing {
	void Scene::cmd_create_blas(
	        VkDevice device, VkPhysicalDevice phys_device, VmaAllocator allocator,
	        std::vector<std::uint32_t> const &indices, std::vector<BuildAccelerationStructure> &build_structures,
	        VkDeviceAddress scratch_address
	) const {
		for (auto idx: indices) {
			VkAccelerationStructureCreateInfoKHR create_info{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
			create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			create_info.size = build_structures[idx].size_info_.accelerationStructureSize;

			build_structures[idx].acc_ = {device, phys_device, allocator, create_info};
		}
	}

	std::vector<Scene::BuildAccelerationStructure> Scene::create_blas(
	        VkPhysicalDevice phys_device, vulkan::CommandBuffer const &command_buffer, VmaAllocator allocator,
	        VkDevice device
	) const {
		std::vector<MeshBlasInput> inputs(meshes_.size());
		std::transform(meshes_.cbegin(), meshes_.cend(), inputs.begin(), [&](Mesh const &mesh) {
			return mesh.to_blas_input();
		});

		std::vector<Scene::BuildAccelerationStructure> build_structures{};
		build_structures.reserve(inputs.size());

		VkDeviceSize acc_str_total_size{0};
		VkDeviceSize max_scratch_size{0};

		for (auto const &input: inputs) {
			VkAccelerationStructureBuildGeometryInfoKHR build_info{
			        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR
			};
			build_info.type          = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			build_info.mode          = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
			build_info.flags         = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
			build_info.geometryCount = input.acc_structure_geom.size();
			build_info.pGeometries   = input.acc_structure_geom.data();

			VkAccelerationStructureBuildRangeInfoKHR const *range_info{};
			range_info = input.acc_structure_build_offset_info.data();

			std::vector<std::uint32_t> max_prim_counts(input.acc_structure_build_offset_info.size());
			std::transform(
			        input.acc_structure_build_offset_info.cbegin(), input.acc_structure_build_offset_info.cend(),
			        max_prim_counts.begin(), [&](auto const &info) { return info.primitiveCount; }
			);
			VkAccelerationStructureBuildSizesInfoKHR size_info{
			        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR
			};

			vulkan::ext::vkGetAccelerationStructureBuildSizesKHR(
			        device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build_info, max_prim_counts.data(),
			        &size_info
			);

			acc_str_total_size += size_info.accelerationStructureSize;
			max_scratch_size = std::max(max_scratch_size, size_info.buildScratchSize);

			build_structures.emplace_back(build_info, size_info, range_info);
		}

		vulkan::Buffer scratch_buffer{
		        device, allocator, max_scratch_size,
		        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 0
		};
		VkDeviceAddress const scratch_device_address{scratch_buffer.get_device_address()};

		std::vector<std::uint32_t> indices{};
		VkDeviceSize               batch_size{};
		constexpr VkDeviceSize     batch_limit{256'000'000};

		for (std::uint32_t idx{}; idx < inputs.size(); ++idx) {
			indices.push_back(idx);
			batch_size += build_structures[idx].size_info_.accelerationStructureSize;

			if (batch_size < batch_limit && idx < inputs.size() - 1) {
				continue;
			}

			cmd_create_blas(device, phys_device, allocator, indices, build_structures, scratch_device_address);

			batch_size = 0;
			indices.clear();
		}

		return build_structures;
	}

	Scene::Scene(
	        vulkan::LogicalDevice const &device, vulkan::CommandPool const &command_pool, VmaAllocator allocator,
	        std::filesystem::path const &path, GltfScene
	) {
		{
			std::string log_message{std::format("Loading GLTF scene \"{}\"", path.string())};
			Logger::get_instance().log(LogLevel::Debug, std::move(log_message));
		}

		fastgltf::Parser parser{};
		auto             data{fastgltf::GltfDataBuffer::FromPath(path)};
		if (data.error() != fastgltf::Error::None) {
			throw std::runtime_error{"Couldn't load GLTF/GLB file"};
		}

		auto asset{parser.loadGltf(data.get(), path.parent_path())};
		if (asset.error() != fastgltf::Error::None) {
			throw std::runtime_error{"Couldn't parse GLTF/GLB file"};
		}
		std::vector<vulkan::UniqueVkFence> fences;
		vulkan::CommandBuffer              command_buffer{command_pool.allocate_command_buffer()};
		command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		fences.reserve(asset->meshes.size());

		for (auto mesh_idx{0}; mesh_idx < asset->meshes.size(); ++mesh_idx) {
			auto const &mesh{asset->meshes[mesh_idx]};

			std::vector<MeshIndex> indices{};
			std::vector<Vertex>    vertices{};

			for (auto &&primitive: mesh.primitives) {
				auto const initial_vertex_idx = vertices.size();

				{
					auto const &index_accessor{asset->accessors[primitive.indicesAccessor.value()]};
					indices.reserve(indices.size() + index_accessor.count);

					fastgltf::iterateAccessor<MeshIndex>(asset.get(), index_accessor, [&](MeshIndex index) {
						indices.push_back(index + initial_vertex_idx);
					});
				}

				{
					auto const &pos_accessor{asset->accessors[primitive.findAttribute("POSITION")->accessorIndex]};
					vertices.resize(vertices.size() + pos_accessor.count);

					fastgltf::iterateAccessorWithIndex<glm::vec3>(
					        asset.get(), pos_accessor,
					        [&](glm::vec3 v, size_t index) {
						        vertices[initial_vertex_idx + index] = {v, glm::vec3{1, 0, 0}, glm::vec2{0, 0}};
					        }
					);
				}

				auto const normals{primitive.findAttribute("NORMAL")};
				if (normals != primitive.attributes.end()) {
					fastgltf::iterateAccessorWithIndex<glm::vec3>(
					        asset.get(), asset->accessors[normals->accessorIndex],
					        [&](glm::vec3 normal, size_t index) { vertices[initial_vertex_idx + index].norm = normal; }
					);
				}

				auto const uv_attr{primitive.findAttribute("TEXCOORD_0")};
				if (uv_attr != primitive.attributes.end()) {
					fastgltf::iterateAccessorWithIndex<glm::vec2>(
					        asset.get(), asset->accessors[uv_attr->accessorIndex],
					        [&](glm::vec2 uv, size_t index) { vertices[initial_vertex_idx + index].uv = uv; }
					);
				}
			}

			std::string debug_msg{std::format(
			        "Uploading mesh \"{}\" with {} indices and {} vertices", mesh.name, indices.size(), vertices.size()
			)};
			Logger::get_instance().log(LogLevel::Debug, std::move(debug_msg));
			meshes_.emplace_back(device.get().device, allocator, command_buffer, indices, vertices);
		}

		auto const blas{create_blas(device.get().physical_device, command_buffer, allocator, device.get().device)};
		// TODO: blas result usage

		vulkan::UniqueVkFence fence{[&] {
			VkFenceCreateInfo fence_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
			VkFence           fence{};
			if (VkResult const result{vkCreateFence(device.get().device, &fence_info, nullptr, &fence)};
			    result != VK_SUCCESS) {
				throw vulkan::VkException{"Could not create fence", result};
			}

			return vulkan::UniqueVkFence{fence, vulkan::VkFenceDestroyer{device.get().device}};
		}()};

		command_buffer.end();
		command_buffer.submit_and_wait(fence.get());
	}
}// namespace raytracing
