#ifndef SRC_VULKAN_RENDER_PASS_H_
#define SRC_VULKAN_RENDER_PASS_H_

#include "constants.h"
#include "src/vulkan/buffer.h"
#include "src/vulkan/command_buffer.h"
#include "src/vulkan/command_pool.h"
#include "src/vulkan/descriptor_pool.h"
#include "src/vulkan/descriptor_set_layout.h"
#include "src/vulkan/fence.h"
#include "src/vulkan/frame_buffer.h"
#include "src/vulkan/image.h"
#include "src/vulkan/image_view.h"
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

	class RenderPass final {
		UniqueVkRenderPass               render_pass_;
		Image                            depth_buffer_;
		UniqueVkImageView                depth_buffer_view_;
		std::vector<UniqueVkFramebuffer> framebuffers_;
		VkExtent2D                       swapchain_extent_;
		Swapchain const                 *swapchain_;
		LogicalDevice const             *logical_device_;

	public:
		RenderPass(Allocator const &allocator, LogicalDevice const &device, Swapchain const &swapchain);

		[[nodiscard]]
		VkRenderPass get() const;

		[[nodiscard]]
		VkFramebuffer get_framebuffer(std::uint32_t image_idx) const;
	};

	class Scene;

	class CommandBufferManager final {
		CommandPool                command_pool_;
		std::vector<CommandBuffer> command_buffers_;

	public:
		CommandBufferManager(LogicalDevice const &device, VkExtent2D extent);

		[[nodiscard]]
		CommandPool const &get_pool() const;

		void submit(std::uint32_t image_idx, VkFence fence, VkSemaphore wait, VkSemaphore signal) const;

		// TODO: this method has an awful lot of parameters...
		void
		record(std::uint32_t current_frame, std::uint32_t image_idx, VkPipeline pipeline, VkExtent2D swapchain_extent,
		       RenderPass const &render_pass, VkDescriptorSet desc_set, VkPipelineLayout pipeline_layout,
		       Scene const &scene) const;
	};

	class SynchronizationManager final {
		std::array<UniqueVkFence, constants::max_frames_in_flight>     fences_;
		std::array<UniqueVkSemaphore, constants::max_frames_in_flight> render_finished_semaphores_;
		std::array<UniqueVkSemaphore, constants::max_frames_in_flight> img_available_semaphores_;
		LogicalDevice const                                           *device_;

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

	struct UniformBufferObject final {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	class Allocator;

	class DescriptorSetManager final {
		DescriptorPool                                               desc_pool_;
		UniqueVkDescriptorSetLayout                                  desc_set_layout_;
		std::array<VkDescriptorSet, constants::max_frames_in_flight> desc_sets_;
		std::array<Buffer, constants::max_frames_in_flight>          uniform_buffers_;
		std::array<MappedBufferPtr, constants::max_frames_in_flight> uniform_buffers_mapped_;
		Image                                                        splorge_image_;
		UniqueVkImageView                                            splorge_image_view_;
		UniqueVkSampler                                              splorge_sampler_;

	public:
		DescriptorSetManager(CommandPool const &command_pool, LogicalDevice const &device, Allocator const &allocator);

		void update(VkExtent2D swapchain_extent, std::uint32_t current_frame) const;

		[[nodiscard]]
		VkDescriptorSetLayout get_layout() const;

		[[nodiscard]]
		VkDescriptorSet get_descriptor_set(std::uint32_t current_frame) const;
	};

	class RenderPassController final {
		RenderPass             render_pass_;
		CommandBufferManager   command_buffer_manager_;
		SynchronizationManager synchronization_manager_;
		QueueManager           queue_manager_;
		DescriptorSetManager   desc_set_manager_;
		LogicalDevice const   *device_;
		Swapchain const       *swapchain_;
		mutable std::uint32_t  current_frame_{0};

	public:
		RenderPassController(LogicalDevice const &device, Allocator const &allocator, Swapchain const &swapchain);

		[[nodiscard]]
		RenderPass const &get_render_pass() const;

		[[nodiscard]]
		DescriptorSetManager const &get_desc_set_manager() const;

		void render(VkPipeline pipeline, VkPipelineLayout pipeline_layout, Scene const &scene) const;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_RENDER_PASS_H_
