#ifndef SRC_VULKAN_RASTERIZER_H_
#define SRC_VULKAN_RASTERIZER_H_

#include "src/vulkan/graphics_pipeline.h"
#include "src/vulkan/shader_module.h"

namespace raytracing::vulkan {
	class LogicalDevice;

	class Swapchain;

	class Rasterizer final {
		ShaderModule     vert_shader_module_;
		ShaderModule     frag_shader_module_;
		GraphicsPipeline pipeline_;

	public:
		Rasterizer(LogicalDevice const &logical_device, Swapchain const &swapchain);

		void render() const;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_RASTERIZER_H_
