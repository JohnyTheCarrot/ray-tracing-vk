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
#include <vulkan/vulkan_core.h>

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
		UniqueVkRenderPass               render_pass_;
		std::vector<UniqueVkFramebuffer> framebuffers_;
		VkExtent2D                       swapchain_extent_;
		Swapchain const                 *swapchain_;
		LogicalDevice const             *logical_device_;

	public:
		RenderPass(LogicalDevice const &device, Swapchain const &swapchain);

		[[nodiscard]]
		VkRenderPass get() const;

		[[nodiscard]]
		VkFramebuffer get_framebuffer(std::uint32_t image_idx) const;
	};

	class CommandBufferManager final {
		CommandPool                command_pool_;
		std::vector<CommandBuffer> command_buffers_;

	public:
		CommandBufferManager(LogicalDevice const &device, VkExtent2D extent);

		void submit(std::uint32_t image_idx, VkFence fence, VkSemaphore wait, VkSemaphore signal) const;

		void
		record(std::uint32_t current_frame, std::uint32_t image_idx, VkPipeline pipeline, VkExtent2D swapchain_extent,
		       RenderPass const &render_pass) const;
	};

	class SynchronizationManager final {
		std::array<UniqueVkFence, max_frames_in_flight>     fences_;
		std::array<UniqueVkSemaphore, max_frames_in_flight> render_finished_semaphores_;
		std::array<UniqueVkSemaphore, max_frames_in_flight> img_available_semaphores_;
		LogicalDevice const                                *device_;

	public:
		explicit SynchronizationManager(LogicalDevice const &device);

		[[nodiscard]]
		VkFence get_fence(std::uint32_t frame_idx) const;

		void wait_for_fence(std::uint32_t frame_idx) const;

		[[nodiscard]]
		VkSemaphore get_render_finished_semaphore(std::uint32_t frame_idx) const;

		[[nodiscard]]
		VkSemaphore get_image_available_semaphore(std::uint32_t frame_idx) const;
	};

	class QueueManager final {
		VkQueue graphics_queue_;
		VkQueue present_queue_;

	public:
		explicit QueueManager(LogicalDevice const &device);

		void queue_presentation(VkSwapchainKHR swapchain, std::uint32_t image_idx, VkSemaphore signal) const;
	};

	class RenderPassController final {
		RenderPass             render_pass_;
		CommandBufferManager   command_buffer_manager_;
		SynchronizationManager synchronization_manager_;
		QueueManager           queue_manager_;
		LogicalDevice const   *device_;
		Swapchain const       *swapchain_;
		mutable std::uint32_t  current_frame_{0};

	public:
		RenderPassController(LogicalDevice const &device, Swapchain const &swapchain);

		[[nodiscard]]
		RenderPass const &get_render_pass() const;

		void render(VkPipeline pipeline) const;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_RENDER_PASS_H_
