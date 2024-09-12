#include "render_pass.h"
#include "VkBootstrap.h"
#include "src/camera.h"
#include "src/diagnostics.h"
#include "src/scene.h"
#include "src/vulkan/allocator.h"
#include "src/vulkan/buffer.h"
#include "src/vulkan/command_buffer.h"
#include "src/vulkan/command_pool.h"
#include "src/vulkan/constants.h"
#include "src/vulkan/frame_buffer.h"
#include "src/vulkan/image_view.h"
#include "src/vulkan/logical_device.h"
#include "src/vulkan/semaphore.h"
#include "src/vulkan/swapchain.h"
#include "src/vulkan/vk_exception.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iterator>
#include <limits>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	VkRenderPassDestroyer::VkRenderPassDestroyer(VkDevice device)
	    : device_{device} {
	}

	void VkRenderPassDestroyer::operator()(VkRenderPass render_pass) const {
		Logger::get_instance().log(LogLevel::Debug, "Destroying render pass");
		vkDestroyRenderPass(device_, render_pass, nullptr);
	}

	RenderPass::RenderPass(Allocator const &allocator, LogicalDevice const &device, Swapchain const &swapchain)
	    : render_pass_{[&] {
		    VkAttachmentDescription color_attachment{};
		    color_attachment.format         = swapchain.get().image_format;
		    color_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
		    color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		    color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		    color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		    color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		    color_attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		    VkAttachmentReference color_attachment_ref{};
		    color_attachment_ref.attachment = 0;
		    color_attachment_ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		    VkAttachmentDescription depth_attachment{};
		    depth_attachment.format         = VK_FORMAT_D32_SFLOAT;
		    depth_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
		    depth_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		    depth_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		    depth_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		    depth_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		    depth_attachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		    VkAttachmentReference depth_attachment_ref{};
		    depth_attachment_ref.attachment = 1;
		    depth_attachment_ref.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		    VkSubpassDescription subpass{};
		    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		    subpass.colorAttachmentCount    = 1;
		    subpass.pColorAttachments       = &color_attachment_ref;
		    subpass.pDepthStencilAttachment = &depth_attachment_ref;

		    std::array const       attachments{color_attachment, depth_attachment};
		    VkRenderPassCreateInfo render_pass_info{};
		    render_pass_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		    render_pass_info.attachmentCount = attachments.size();
		    render_pass_info.pAttachments    = attachments.data();
		    render_pass_info.subpassCount    = 1;
		    render_pass_info.pSubpasses      = &subpass;

		    VkSubpassDependency dep{};
		    dep.srcSubpass = VK_SUBPASS_EXTERNAL;
		    dep.dstSubpass = 0;
		    dep.srcStageMask =
		            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		    dep.srcAccessMask = 0;
		    dep.dstStageMask =
		            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		    render_pass_info.dependencyCount = 1;
		    render_pass_info.pDependencies   = &dep;

		    VkRenderPass render_pass{};
		    if (VkResult const result{vkCreateRenderPass(device.get(), &render_pass_info, nullptr, &render_pass)};
		        result != VK_SUCCESS) {
			    throw VkException{"Could not create render pass", result};
		    }

		    return UniqueVkRenderPass{render_pass, VkRenderPassDestroyer{device.get()}};
	    }()}
	    , depth_buffer_{device.create_image(
	              allocator, swapchain.get().extent.width, swapchain.get().extent.height, VK_FORMAT_D32_SFLOAT,
	              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
	      )}
	    , depth_buffer_view_{depth_buffer_.create_image_view(VK_IMAGE_ASPECT_DEPTH_BIT)} {
		auto const &image_views{swapchain.get_views()};

		framebuffers_.reserve(image_views.size());

		std::transform(
		        image_views.cbegin(), image_views.cend(), std::back_inserter(framebuffers_),
		        [&](UniqueVkImageView const &image_view) {
			        std::array const attachments{image_view.get(), depth_buffer_view_.get()};

			        VkFramebufferCreateInfo framebuffer_create_info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
			        framebuffer_create_info.renderPass      = render_pass_.get();
			        framebuffer_create_info.attachmentCount = attachments.size();
			        framebuffer_create_info.pAttachments    = attachments.data();
			        framebuffer_create_info.width           = swapchain.get().extent.width;
			        framebuffer_create_info.height          = swapchain.get().extent.height;
			        framebuffer_create_info.layers          = 1;

			        VkFramebuffer framebuffer{};
			        if (VkResult const result{
			                    vkCreateFramebuffer(device.get(), &framebuffer_create_info, nullptr, &framebuffer)
			            };
			            result != VK_SUCCESS) {
				        throw VkException{"Couldn't create framebuffer", result};
			        }

			        return UniqueVkFramebuffer{framebuffer, VkFramebufferDestroyer{device.get()}};
		        }
		);
	}

	VkRenderPass RenderPass::get() const {
		return render_pass_.get();
	}

	VkFramebuffer RenderPass::get_framebuffer(std::uint32_t image_idx) const {
		return framebuffers_[image_idx].get();
	}

	CommandBufferManager::CommandBufferManager(LogicalDevice const &device, VkExtent2D extent)
	    : command_pool_{vkb::QueueType::graphics, device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT}
	    , command_buffers_{command_pool_.allocate_command_buffers(constants::max_frames_in_flight)} {
	}

	void CommandBufferManager::record(
	        std::uint32_t current_frame, std::uint32_t image_idx, VkPipeline pipeline, VkExtent2D swapchain_extent,
	        RenderPass const &render_pass, VkDescriptorSet desc_set, VkPipelineLayout pipeline_layout,
	        Scene const &scene
	) const {
		auto const &command_buffer{command_buffers_[current_frame]};
		command_buffer.reset();

		command_buffer.begin(0);

		VkRenderPassBeginInfo render_pass_info{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
		render_pass_info.renderPass        = render_pass.get();
		render_pass_info.framebuffer       = render_pass.get_framebuffer(image_idx);
		render_pass_info.renderArea.offset = {0, 0};
		render_pass_info.renderArea.extent = swapchain_extent;

		std::array clear_values{VkClearValue{.color = {0.f, 0.f, 0.f, 1.f}}, VkClearValue{.depthStencil = {1.f, 0}}};
		render_pass_info.clearValueCount = clear_values.size();
		render_pass_info.pClearValues    = clear_values.data();

		vkCmdBeginRenderPass(command_buffer.get(), &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(command_buffer.get(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		VkViewport viewport{};
		viewport.x        = 0.f;
		viewport.y        = 0.f;
		viewport.width    = static_cast<float>(swapchain_extent.width);
		viewport.height   = static_cast<float>(swapchain_extent.height);
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;
		vkCmdSetViewport(command_buffer.get(), 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = swapchain_extent;
		vkCmdSetScissor(command_buffer.get(), 0, 1, &scissor);

		VkDeviceSize offsets[]{0};
		scene.rasterizer_draw(command_buffer.get(), pipeline_layout, desc_set);

		vkCmdEndRenderPass(command_buffer.get());

		command_buffer.end();
	}

	void
	CommandBufferManager::submit(std::uint32_t image_idx, VkFence fence, VkSemaphore wait, VkSemaphore signal) const {
		VkPipelineStageFlags wait_stage{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

		VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores    = &wait;
		submit_info.pWaitDstStageMask  = &wait_stage;
		submit_info.commandBufferCount = 1;

		auto const     &cmd_buf{command_buffers_[image_idx]};
		VkCommandBuffer cmd_buf_raw{cmd_buf.get()};
		submit_info.pCommandBuffers = &cmd_buf_raw;

		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores    = &signal;

		cmd_buf.submit(fence, submit_info);
	}

	SynchronizationManager::SynchronizationManager(LogicalDevice const &device)
	    : fences_{device.create_fence(VK_FENCE_CREATE_SIGNALED_BIT), device.create_fence(VK_FENCE_CREATE_SIGNALED_BIT)}
	    , render_finished_semaphores_{device.create_semaphore(), device.create_semaphore()}
	    , img_available_semaphores_{device.create_semaphore(), device.create_semaphore()}
	    , device_{&device} {
	}

	void SynchronizationManager::wait_for_fence(std::uint32_t frame_idx) const {
		VkFence fence{fences_[frame_idx].get()};
		vkWaitForFences(device_->get(), 1, &fence, 1, std::numeric_limits<std::uint64_t>::max());
		vkResetFences(device_->get(), 1, &fence);
	}

	VkFence SynchronizationManager::get_fence(std::uint32_t frame_idx) const {
		return fences_[frame_idx].get();
	}

	VkSemaphore SynchronizationManager::get_render_finished_semaphore(std::uint32_t frame_idx) const {
		return render_finished_semaphores_[frame_idx].get();
	}

	VkSemaphore SynchronizationManager::get_image_available_semaphore(std::uint32_t frame_idx) const {
		return img_available_semaphores_[frame_idx].get();
	}

	QueueManager::QueueManager(LogicalDevice const &device)
	    : graphics_queue_{device.get_queue(vkb::QueueType::graphics)}
	    , present_queue_{device.get_queue(vkb::QueueType::present)} {
	}

	void QueueManager::queue_presentation(VkSwapchainKHR swapchain, std::uint32_t image_idx, VkSemaphore signal) const {
		VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores    = &signal;

		present_info.swapchainCount = 1;
		present_info.pSwapchains    = &swapchain;
		present_info.pImageIndices  = &image_idx;

		vkQueuePresentKHR(present_queue_, &present_info);
	}

	constexpr VkDescriptorSetLayoutBinding ubo_layout_binding{
	        .binding            = 0,
	        .descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	        .descriptorCount    = 1,
	        .stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
	        .pImmutableSamplers = VK_NULL_HANDLE
	};

	std::vector const bindings{ubo_layout_binding};

	DescriptorSetManager::DescriptorSetManager(LogicalDevice const &device, Allocator const &allocator)
	    : desc_pool_{device.get(), bindings, constants::max_frames_in_flight}
	    , desc_set_layout_{device.create_descriptor_set_layout({}, std::span{bindings})}
	    , desc_sets_{desc_pool_.create_descriptor_set(desc_set_layout_.get()), desc_pool_.create_descriptor_set(desc_set_layout_.get())} 
	    , uniform_buffers_{
	              Buffer{device.get().device, allocator.get(), sizeof(UniformBufferObject),
	                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT},
	              Buffer{device.get().device, allocator.get(), sizeof(UniformBufferObject),
	                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT}
	      }
	, uniform_buffers_mapped_{uniform_buffers_[0].map_memory(), uniform_buffers_[1].map_memory()} {
		for (int i{}; i < constants::max_frames_in_flight; ++i) {
			VkDescriptorBufferInfo buffer_info{};
			buffer_info.buffer = uniform_buffers_[i].get();
			buffer_info.offset = 0;
			buffer_info.range  = sizeof(UniformBufferObject);

			VkWriteDescriptorSet descriptor_write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
			descriptor_write.dstSet          = desc_sets_[i];
			descriptor_write.dstBinding      = 0;
			descriptor_write.dstArrayElement = 0;

			descriptor_write.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptor_write.descriptorCount = 1;

			descriptor_write.pBufferInfo = &buffer_info;
			vkUpdateDescriptorSets(device.get(), 1, &descriptor_write, 0, nullptr);
		}
	}

	void DescriptorSetManager::update(VkExtent2D swapchain_extent, std::uint32_t current_frame) const {
		UniformBufferObject ubo{glm::mat4{1.f}, glm::mat4{1.f}};

		static auto startTime = std::chrono::high_resolution_clock::now();

		auto  currentTime = std::chrono::high_resolution_clock::now();
		float time        = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		ubo.model = glm::scale(glm::mat4{1.f}, glm::vec3{0.001f});
		ubo.view  = Camera::get_instance().get_mat();
		ubo.proj  = glm::perspective(
                glm::radians(45.f),
                static_cast<float>(swapchain_extent.width) / static_cast<float>(swapchain_extent.height), 0.1f, 10000.f
        );
		ubo.proj[1][1] *= -1;
		memcpy(uniform_buffers_mapped_[current_frame].get_mapped_ptr(), &ubo, sizeof(ubo));
	}

	VkDescriptorSetLayout DescriptorSetManager::get_layout() const {
		return desc_set_layout_.get();
	}

	VkDescriptorSet DescriptorSetManager::get_descriptor_set(std::uint32_t current_frame) const {
		return desc_sets_[current_frame];
	}

	RenderPassController::RenderPassController(
	        LogicalDevice const &device, Allocator const &allocator, Swapchain const &swapchain
	)
	    : render_pass_{allocator, device, swapchain}
	    , command_buffer_manager_{device, swapchain.get().extent}
	    , synchronization_manager_{device}
	    , queue_manager_{device}
	    , desc_set_manager_{device, allocator}
	    , device_{&device}
	    , swapchain_{&swapchain} {
	}

	RenderPass const &RenderPassController::get_render_pass() const {
		return render_pass_;
	}

	DescriptorSetManager const &RenderPassController::get_desc_set_manager() const {
		return desc_set_manager_;
	}

	void RenderPassController::render(VkPipeline pipeline, VkPipelineLayout pipeline_layout, Scene const &scene) const {
		desc_set_manager_.update(swapchain_->get().extent, current_frame_);

		synchronization_manager_.wait_for_fence(current_frame_);

		VkSemaphore const img_available_semaphore{synchronization_manager_.get_image_available_semaphore(current_frame_)
		};
		std::uint32_t     image_idx{};
		if (VkResult const result{vkAcquireNextImageKHR(
		            device_->get(), swapchain_->get(), std::numeric_limits<std::uint64_t>::max(),
		            img_available_semaphore, VK_NULL_HANDLE, &image_idx
		    )};
		    result != VK_SUCCESS) {
			throw VkException{"Failed to acquire next image from swapchain", result};
		}

		command_buffer_manager_.record(
		        current_frame_, image_idx, pipeline, swapchain_->get().extent, render_pass_,
		        desc_set_manager_.get_descriptor_set(current_frame_), pipeline_layout, scene
		);

		VkSemaphore const semaphore{synchronization_manager_.get_image_available_semaphore(current_frame_)};
		VkSemaphore const signal_semaphore{synchronization_manager_.get_render_finished_semaphore(current_frame_)};
		VkFence const     fence{synchronization_manager_.get_fence(current_frame_)};
		command_buffer_manager_.submit(current_frame_, fence, semaphore, signal_semaphore);
		queue_manager_.queue_presentation(swapchain_->get().swapchain, image_idx, signal_semaphore);

		current_frame_ = (current_frame_ + 1) % constants::max_frames_in_flight;
	}
}// namespace raytracing::vulkan
