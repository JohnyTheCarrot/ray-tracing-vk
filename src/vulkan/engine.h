#ifndef SRC_VULKAN_ENGINE_H_
#define SRC_VULKAN_ENGINE_H_

#include "src/vulkan/allocator.h"
#include "src/vulkan/command_pool.h"
#include "src/vulkan/graphics_pipeline.h"
#include "src/vulkan/logical_device.h"
#include "src/vulkan/phys_device.h"
#include "src/vulkan/shader_module.h"
#include "src/vulkan/swapchain.h"
#include "src/vulkan/vkb_raii.h"
#include "src/window.h"

namespace raytracing::vulkan {
	class Engine final {
		UniqueVkbInstance instance_;
		Window            window_;
		vulkan::Surface  *surface_;
		PhysicalDevice    physical_device_;
		LogicalDevice     logical_device_;
		CommandPool       command_pool_;
		Swapchain         swapchain_;
		Allocator         allocator_;
		ShaderModule      vert_shader_module_;
		ShaderModule      frag_shader_module_;
		GraphicsPipeline  graphics_pipeline_;

	public:
		explicit Engine(std::string_view app_name);

		[[nodiscard]]
		VmaAllocator get_allocator() const;

		[[nodiscard]]
		LogicalDevice &get_logical_device();

		[[nodiscard]]
		LogicalDevice const &get_logical_device() const;

		[[nodiscard]]
		CommandPool &get_command_pool();

		[[nodiscard]]
		CommandPool const &get_command_pool() const;

		void main_loop();

		void copy_buffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) const;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_ENGINE_H_
