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
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/matrix.hpp>
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

			build_structures[idx].acc_ = {device, allocator, create_info};
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

	vulkan::AccelerationStructure Scene::create_tlas(
	        vulkan::LogicalDevice const &device, VmaAllocator allocator, vulkan::CommandBuffer const &command_buffer,
	        std::vector<BuildAccelerationStructure> const &blas, std::vector<MeshInstance> const &mesh_instances
	) {
		std::vector<VkAccelerationStructureInstanceKHR> instances{};
		instances.reserve(mesh_instances.size());

		for (auto const &mesh_instance: mesh_instances) {
			VkAccelerationStructureInstanceKHR instance{};
			instance.transform = [&] {
				glm::mat4 const      transposed{glm::transpose(mesh_instance.model_matrix_)};
				VkTransformMatrixKHR result{};
				memcpy(&result, &transposed, sizeof(VkTransformMatrixKHR));

				return result;
			}();
			instance.instanceCustomIndex            = mesh_instance.mesh_idx_;
			instance.accelerationStructureReference = [&] {
				VkAccelerationStructureDeviceAddressInfoKHR address_info{
				        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR
				};
				address_info.accelerationStructure = blas[mesh_instance.mesh_idx_].acc_.value().get_acc();

				return vulkan::ext::vkGetAccelerationStructureDeviceAddressKHR(device.get().device, &address_info);
			}();
			instance.flags                                  = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
			instance.mask                                   = 0xFF;
			instance.instanceShaderBindingTableRecordOffset = 0;
			instances.emplace_back(instance);
		}

		std::uint32_t  instance_count{static_cast<std::uint32_t>(instances.size())};
		vulkan::Buffer instances_buffer{
		        device.get().device, allocator, std::span<VkAccelerationStructureInstanceKHR>{instances},
		        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
		                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
		};

		VkMemoryBarrier barrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;

		vkCmdPipelineBarrier(
		        command_buffer.get(), VK_PIPELINE_STAGE_TRANSFER_BIT,
		        VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr
		);

		// cmd
		VkAccelerationStructureGeometryInstancesDataKHR instances_vk{
		        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR
		};
		instances_vk.data.deviceAddress = instances_buffer.get_device_address();

		VkAccelerationStructureGeometryKHR top_acc_structure_geom{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR
		};

		top_acc_structure_geom.geometryType       = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		top_acc_structure_geom.geometry.instances = instances_vk;

		VkAccelerationStructureBuildGeometryInfoKHR build_geometry_info{
		        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR
		};
		build_geometry_info.flags                    = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		build_geometry_info.geometryCount            = 1;
		build_geometry_info.pGeometries              = &top_acc_structure_geom;
		build_geometry_info.mode                     = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		build_geometry_info.type                     = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		build_geometry_info.srcAccelerationStructure = VK_NULL_HANDLE;

		VkAccelerationStructureBuildSizesInfoKHR size_info{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR
		};
		vulkan::ext::vkGetAccelerationStructureBuildSizesKHR(
		        device.get().device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build_geometry_info,
		        &instance_count, &size_info
		);

		VkAccelerationStructureCreateInfoKHR create_info{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
		create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		create_info.size = size_info.accelerationStructureSize;

		auto const scratch_alignment{
		        device.get_phys().get_as_properties().minAccelerationStructureScratchOffsetAlignment
		};

		vulkan::AccelerationStructure tlas{device.get().device, allocator, create_info};
		vulkan::Buffer                scratch_buffer{
                device.get().device,
                allocator,
                size_info.buildScratchSize,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                0,
                scratch_alignment
        };
		VkDeviceAddress scratch_buff_addr{scratch_buffer.get_device_address()};

		build_geometry_info.dstAccelerationStructure  = tlas.get_acc();
		build_geometry_info.scratchData.deviceAddress = scratch_buff_addr;

		VkAccelerationStructureBuildRangeInfoKHR build_offset_info{instance_count, 0, 0, 0};
		auto const                              *build_offset_info_ptr{&build_offset_info};

		vulkan::ext::vkCmdBuildAccelerationStructuresKHR(
		        device.get().device, command_buffer.get(), 1, &build_geometry_info, &build_offset_info_ptr
		);

		return tlas;
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

		for (auto const &node: asset->nodes) {
			SceneNode scene_node{};

			if (node.meshIndex.has_value()) {
				scene_node.mesh_idx_ = node.meshIndex.value();
			}

			glm::mat4 glm_mat{};

			if (std::holds_alternative<fastgltf::math::fmat4x4>(node.transform)) {
				auto const mat{std::get<fastgltf::math::fmat4x4>(node.transform)};
				memcpy(&glm_mat, mat.data(), sizeof(mat));
			} else {
				auto const trs{std::get<fastgltf::TRS>(node.transform)};
				glm::vec3  trans{trs.translation.x(), trs.translation.y(), trs.translation.z()};
				glm::quat  rot{trs.rotation.w(), trs.rotation.x(), trs.rotation.y(), trs.rotation.z()};
				glm::vec3  scale{trs.scale.x(), trs.scale.y(), trs.scale.z()};

				glm::mat4 trans_mat{glm::translate(glm::mat4{1.f}, trans)};
				glm::mat4 rot_mat{glm::toMat4(rot)};
				glm::mat4 scale_mat{glm::scale(glm::mat4{1.f}, scale)};

				glm_mat = trans_mat * rot_mat * scale_mat;
			}

			scene_node.local_matrix_ = glm_mat;
			nodes_.emplace_back(scene_node);
		}

		for (std::size_t idx{}; idx < asset->nodes.size(); ++idx) {
			auto       &node{nodes_.at(idx)};
			auto const &gltf_node{asset->nodes[idx]};

			for (auto child_idx: gltf_node.children) {
				auto &child_node{nodes_[child_idx]};

				child_node.local_matrix_ = node.local_matrix_ * child_node.local_matrix_;
			}
		}

		std::vector<MeshInstance> mesh_instances{};
		mesh_instances.reserve(nodes_.size());
		for (auto const &node: nodes_) {
			if (!node.mesh_idx_.has_value())
				continue;

			mesh_instances.emplace_back(node.local_matrix_, node.mesh_idx_.value());
		}

		blas_ = create_blas(device.get().physical_device, command_buffer, allocator, device.get().device);
		tlas_ = create_tlas(device, allocator, command_buffer, blas_, mesh_instances);

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
