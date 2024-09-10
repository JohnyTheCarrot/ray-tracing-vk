#ifndef SRC_VULKAN_GRAPHICS_PIPELINE_H_
#define SRC_VULKAN_GRAPHICS_PIPELINE_H_

#include "pipeline_layout.h"
#include "src/vulkan/render_pass.h"
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

struct VkDevice_T;
using VkDevice = VkDevice_T *;

struct VkPipeline_T;
using VkPipeline = VkPipeline_T *;

namespace raytracing::vulkan {
	class Swapchain;

	class LogicalDevice;

	class VkPipelineDestroyer final {
		VkDevice device_;

	public:
		explicit VkPipelineDestroyer(VkDevice device);

		void operator()(VkPipeline pipeline) const;
	};

	using UniqueVkPipeline = std::unique_ptr<VkPipeline_T, VkPipelineDestroyer>;

	class GraphicsPipeline final {
		RenderPass       render_pass_;
		PipelineLayout   pipeline_layout_;
		UniqueVkPipeline pipeline_;

	public:
		GraphicsPipeline(
		        LogicalDevice const &device, Swapchain const &swapchain,
		        std::vector<VkPipelineShaderStageCreateInfo> const &shader_stages
		);

		void render();
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_GRAPHICS_PIPELINE_H_
