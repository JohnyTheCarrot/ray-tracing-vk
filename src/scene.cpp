#include "scene.h"
#include "src/diagnostics.h"
#include "src/vulkan/command_pool.h"
#include "src/vulkan/logical_device.h"
#include "src/vulkan/vk_exception.h"
#include "src/vulkan/vkb_raii.h"
#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <format>
#include <limits>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace raytracing {
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
			meshes_.emplace_back(allocator, command_buffer, std::move(indices), std::move(vertices));
		}

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
