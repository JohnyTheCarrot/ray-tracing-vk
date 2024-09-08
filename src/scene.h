#ifndef SRC_MODEL_H_
#define SRC_MODEL_H_

#include "src/mesh.h"
#include <filesystem>

namespace raytracing {
	struct GltfScene final {};

	namespace vulkan {
		class LogicalDevice;

		class CommandPool;
	}// namespace vulkan

	class Scene final {
		std::vector<Mesh> meshes_;

	public:
		Scene(vulkan::LogicalDevice const &device, vulkan::CommandPool const &command_pool, VmaAllocator allocator,
		      std::filesystem::path const &path, GltfScene);
	};
}// namespace raytracing

#endif//  SRC_MODEL_H_
