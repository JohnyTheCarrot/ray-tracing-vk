#ifndef SRC_VULKAN_RENDER_PASS_H_
#define SRC_VULKAN_RENDER_PASS_H_

#include "src/vulkan/command_buffer.h"
#include "src/vulkan/command_pool.h"
#include "src/vulkan/fence.h"
#include "src/vulkan/frame_buffer.h"
#include "src/vulkan/semaphore.h"
#include <array>
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

	constexpr int max_frames_in_flight{2};

	class RenderPass final {
		UniqueVkRenderPass                                  render_pass_;
		std::vector<UniqueVkFramebuffer>                    framebuffers_;
		CommandPool                                         command_pool_;
		VkExtent2D                                          swapchain_extent_;
		std::array<CommandBuffer, max_frames_in_flight>     command_buffers_;
		std::array<UniqueVkFence, max_frames_in_flight>     fences_;
		std::array<UniqueVkSemaphore, max_frames_in_flight> render_finished_semaphores_;
		std::array<UniqueVkSemaphore, max_frames_in_flight> img_available_semaphores_;
		Swapchain const                                    *swapchain_;
		LogicalDevice const                                *device_;
		VkQueue                                             graphics_queue_;
		VkQueue                                             present_queue_;
		std::uint32_t                                       current_frame_{0};

	public:
		RenderPass(LogicalDevice const &device, Swapchain const &swapchain);

		[[nodiscard]]
		VkRenderPass get() const;

		void record_cmd_buff(std::uint32_t image_idx, VkPipeline pipeline) const;

		void render(VkPipeline pipeline);
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_RENDER_PASS_H_
