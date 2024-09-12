#ifndef SRC_VULKAN_ENGINE_H_
#define SRC_VULKAN_ENGINE_H_

#include "src/scene.h"
#include "src/vulkan/device_manager.h"
#include "src/vulkan/graphics_pipeline.h"
#include "src/vulkan/swapchain.h"
#include "src/vulkan/vk_core.h"
#include <filesystem>

namespace raytracing::vulkan {
	class CommandPool;

	class LogicalDevice;

	class Engine final {
		VulkanCore    core_;
		DeviceManager device_manager_;

		Swapchain        swapchain_;
		GraphicsPipeline rasterizer_;
		Scene            scene_;

	public:
		explicit Engine(std::string_view app_name);

		[[nodiscard]]
		DeviceManager const &get_device_manager() const;

		[[nodiscard]]
		Scene load_scene(std::filesystem::path const &path, SceneFormat format);

		void main_loop();
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_ENGINE_H_
