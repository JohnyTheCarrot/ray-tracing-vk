#ifndef SRC_VULKAN_RENDER_PASS_H_
#define SRC_VULKAN_RENDER_PASS_H_

#include "src/vulkan/command_buffer.h"
#include "src/vulkan/command_pool.h"
#include "src/vulkan/frame_buffer.h"
#include "src/vulkan/semaphore.h"
#include "src/vulkan/vkb_raii.h"
#include <cstdint>
#include <memory>
#include <vector>

struct VkDevice_T;
using VkDevice = VkDevice_T *;

struct VkRenderPass_T;
using VkRenderPass = VkRenderPass_T *;

namespace raytracing::vulkan {
	class Swapchain;

	class LogicalDevice;

	class VkRenderPassDestroyer final {
		VkDevice device_;

	public:
		explicit VkRenderPassDestroyer(VkDevice device);

		void operator()(VkRenderPass render_pass) const;
	};

	using UniqueVkRenderPass = std::unique_ptr<VkRenderPass_T, VkRenderPassDestroyer>;

	class RenderPass final {
		UniqueVkRenderPass               render_pass_;
		std::vector<UniqueVkFramebuffer> framebuffers_;
		CommandPool                      command_pool_;
		CommandBuffer                    command_buffer_;
		VkExtent2D                       swapchain_extent_;
		UniqueVkFence                    fence_;
		UniqueVkSemaphore                render_finished_semaphore_;
		UniqueVkSemaphore                img_available_semaphore_;
		Swapchain const                 *swapchain_;
		LogicalDevice const             *device_;
		VkQueue                          graphics_queue_;
		VkQueue                          present_queue_;

	public:
		RenderPass(LogicalDevice const &device, Swapchain const &swapchain);

		[[nodiscard]]
		VkRenderPass get() const;

		void record_cmd_buff(std::uint32_t image_idx, VkPipeline pipeline) const;

		void render(VkPipeline pipeline) const;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_RENDER_PASS_H_
