#include "render_pass.h"
#include "VkBootstrap.h"
#include "src/diagnostics.h"
#include "src/vulkan/command_buffer.h"
#include "src/vulkan/command_pool.h"
#include "src/vulkan/frame_buffer.h"
#include "src/vulkan/image_view.h"
#include "src/vulkan/logical_device.h"
#include "src/vulkan/semaphore.h"
#include "src/vulkan/swapchain.h"
#include "src/vulkan/vk_exception.h"
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <limits>
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	VkRenderPassDestroyer::VkRenderPassDestroyer(VkDevice device)
	    : device_{device} {
	}

	void VkRenderPassDestroyer::operator()(VkRenderPass render_pass) const {
		Logger::get_instance().log(LogLevel::Debug, "Destroying render pass");
		vkDestroyRenderPass(device_, render_pass, nullptr);
	}

	RenderPass::RenderPass(LogicalDevice const &device, Swapchain const &swapchain)
	    : render_pass_{[&] {
		    VkAttachmentDescription color_attachment{};
		    color_attachment.format  = swapchain.get().image_format;
		    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		    color_attachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
		    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		    color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		    color_attachment.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		    VkAttachmentReference color_attachment_ref{};
		    color_attachment_ref.attachment = 0;
		    color_attachment_ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		    VkSubpassDescription subpass{};
		    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		    subpass.colorAttachmentCount = 1;
		    subpass.pColorAttachments    = &color_attachment_ref;

		    VkRenderPassCreateInfo render_pass_info{};
		    render_pass_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		    render_pass_info.attachmentCount = 1;
		    render_pass_info.pAttachments    = &color_attachment;
		    render_pass_info.subpassCount    = 1;
		    render_pass_info.pSubpasses      = &subpass;

		    VkSubpassDependency dep{};
		    dep.srcSubpass    = VK_SUBPASS_EXTERNAL;
		    dep.dstSubpass    = 0;
		    dep.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		    dep.srcAccessMask = 0;
		    dep.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		    render_pass_info.dependencyCount = 1;
		    render_pass_info.pDependencies   = &dep;

		    VkRenderPass render_pass{};
		    if (VkResult const result{vkCreateRenderPass(device.get(), &render_pass_info, nullptr, &render_pass)};
		        result != VK_SUCCESS) {
			    throw VkException{"Could not create render pass", result};
		    }

		    return UniqueVkRenderPass{render_pass, VkRenderPassDestroyer{device.get()}};
	    }()}
	    , command_pool_{device.get_queue_index(vkb::QueueType::graphics), device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT}
	    , command_buffer_{command_pool_.allocate_command_buffer()}
	    , swapchain_extent_{swapchain.get().extent}
	    , fence_{[&] {
		    VkFenceCreateInfo fence_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
		    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		    VkFence fence{};
		    if (VkResult const result{vkCreateFence(device.get().device, &fence_info, nullptr, &fence)};
		        result != VK_SUCCESS) {
			    throw vulkan::VkException{"Could not create fence", result};
		    }

		    return vulkan::UniqueVkFence{fence, vulkan::VkFenceDestroyer{device.get().device}};
	    }()}
	    , render_finished_semaphore_{create_semaphore(device.get())}
	    , img_available_semaphore_{create_semaphore(device.get())}
	    , swapchain_{&swapchain}
	    , device_{&device}
	    , graphics_queue_{device.get_queue(vkb::QueueType::graphics)}
	    , present_queue_{device.get_queue(vkb::QueueType::present)} {
		auto const &image_views{swapchain.get_views()};

		framebuffers_.reserve(image_views.size());
		std::transform(
		        image_views.cbegin(), image_views.cend(), std::back_inserter(framebuffers_),
		        [&](UniqueVkImageView const &image_view) {
			        auto const attachment{image_view.get()};

			        VkFramebufferCreateInfo framebuffer_create_info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
			        framebuffer_create_info.renderPass      = render_pass_.get();
			        framebuffer_create_info.attachmentCount = 1;
			        framebuffer_create_info.pAttachments    = &attachment;
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

	// TODO: look into making the pipeline a member instead
	void RenderPass::record_cmd_buff(std::uint32_t image_idx, VkPipeline pipeline) const {
		command_buffer_.begin(0);

		VkRenderPassBeginInfo render_pass_info{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
		render_pass_info.renderPass        = render_pass_.get();
		render_pass_info.framebuffer       = framebuffers_.at(image_idx).get();
		render_pass_info.renderArea.offset = {0, 0};
		render_pass_info.renderArea.extent = swapchain_extent_;

		VkClearValue clear_color{{{0.f, 0.f, 0.f, 1.f}}};
		render_pass_info.clearValueCount = 1;
		render_pass_info.pClearValues    = &clear_color;

		vkCmdBeginRenderPass(command_buffer_.get(), &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(command_buffer_.get(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		VkViewport viewport{};
		viewport.x        = 0.f;
		viewport.y        = 0.f;
		viewport.width    = static_cast<float>(swapchain_extent_.width);
		viewport.height   = static_cast<float>(swapchain_extent_.height);
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;
		vkCmdSetViewport(command_buffer_.get(), 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = swapchain_extent_;
		vkCmdSetScissor(command_buffer_.get(), 0, 1, &scissor);

		vkCmdDraw(command_buffer_.get(), 3, 1, 0, 0);

		vkCmdEndRenderPass(command_buffer_.get());

		command_buffer_.end();
	}

	void RenderPass::render(VkPipeline pipeline) const {
		VkFence fence{fence_.get()};
		vkWaitForFences(device_->get(), 1, &fence, 1, std::numeric_limits<std::uint64_t>::max());
		vkResetFences(device_->get(), 1, &fence);

		std::uint32_t image_idx{};
		if (VkResult const result{vkAcquireNextImageKHR(
		            device_->get(), swapchain_->get(), std::numeric_limits<std::uint64_t>::max(),
		            img_available_semaphore_.get(), VK_NULL_HANDLE, &image_idx
		    )};
		    result != VK_SUCCESS) {
			throw VkException{"Failed to acquire next image from swapchain", result};
		}

		vkResetCommandBuffer(command_buffer_.get(), 0);
		record_cmd_buff(image_idx, pipeline);

		VkSemaphore          semaphore{img_available_semaphore_.get()};
		VkPipelineStageFlags wait_stage{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		VkCommandBuffer      command_buffer{command_buffer_.get()};

		VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores    = &semaphore;
		submit_info.pWaitDstStageMask  = &wait_stage;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers    = &command_buffer;

		VkSemaphore signal_semaphore{render_finished_semaphore_.get()};
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores    = &signal_semaphore;

		command_buffer_.submit(fence_.get(), submit_info);

		VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores    = &signal_semaphore;

		VkSwapchainKHR swapchain{swapchain_->get()};
		present_info.swapchainCount = 1;
		present_info.pSwapchains    = &swapchain;
		present_info.pImageIndices  = &image_idx;

		vkQueuePresentKHR(present_queue_, &present_info);
	}
}// namespace raytracing::vulkan
