#ifndef SRC_VULKAN_GRAPHICS_PIPELINE_H_
#define SRC_VULKAN_GRAPHICS_PIPELINE_H_

#include "src/vulkan/render_pass.h"
#include "src/vulkan/shader_module.h"
#include <memory>
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	class Swapchain;

	class LogicalDevice;

	class Allocator;

	class VkPipelineDestroyer final {
		VkDevice device_;

	public:
		explicit VkPipelineDestroyer(VkDevice device);

		void operator()(VkPipeline pipeline) const;
	};

	using UniqueVkPipeline = std::unique_ptr<VkPipeline_T, VkPipelineDestroyer>;

	class Scene;

	class GraphicsPipeline final {
		RenderPassController   render_pass_;
		ShaderModule           vert_shader_module_;
		ShaderModule           frag_shader_module_;
		UniqueVkPipelineLayout pipeline_layout_;
		UniqueVkPipeline       pipeline_;

	public:
		GraphicsPipeline(LogicalDevice const &device, Allocator const &allocator, Swapchain const &swapchain);

		void render(Scene const &scene) const;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_GRAPHICS_PIPELINE_H_
