#include "command_buffer.h"
#include "src/diagnostics.h"
#include "src/vulkan/vk_exception.h"
#include "vkb_raii.h"
#include <cassert>
#include <limits>
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	CommandBufferDestroyer::CommandBufferDestroyer(VkDevice device, VkCommandPool cmd_pool)
	    : device_{device}
	    , cmd_pool_{cmd_pool} {
	}

	void CommandBufferDestroyer::operator()(VkCommandBuffer buffer) {
		Logger::get_instance().log(LogLevel::Debug, "Destroying command buffer");
		vkFreeCommandBuffers(device_, cmd_pool_, 1, &buffer);
	}

	CommandBuffer::CommandBuffer(UniqueVkCommandBuffer &&buffer, VkDevice device, VkQueue queue)
	    : cmd_buffer_{std::move(buffer)}
	    , queue_{queue}
	    , device_{device} {
	}

	void CommandBuffer::begin(VkCommandBufferUsageFlags flags) const {
		VkCommandBufferBeginInfo begin_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
		begin_info.flags = flags;

		vkBeginCommandBuffer(cmd_buffer_.get(), &begin_info);
	}

	void CommandBuffer::end() const {
		vkEndCommandBuffer(cmd_buffer_.get());
	}

	void CommandBuffer::reset() const {
		vkResetCommandBuffer(cmd_buffer_.get(), 0);
	}

	void CommandBuffer::submit(VkFence optional_fence, VkSubmitInfo submit_info) const {
		auto const buff{cmd_buffer_.get()};

		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers    = &buff;

		if (VkResult const result{vkQueueSubmit(queue_, 1, &submit_info, optional_fence)}; result != VK_SUCCESS) {
			throw VkException{"Failed to submit to queue", result};
		}
	}

	void CommandBuffer::submit_and_wait(VkFence mandatory_fence, VkSubmitInfo submit_info) const {
		UniqueVkFence unique_fence{[&] {
			if (mandatory_fence != VK_NULL_HANDLE) {
				return UniqueVkFence{VK_NULL_HANDLE, VkFenceDestroyer{VK_NULL_HANDLE}};
			}

			VkFenceCreateInfo fence_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
			VkFence           fence{};
			if (VkResult const result{vkCreateFence(device_, &fence_info, nullptr, &fence)}; result != VK_SUCCESS) {
				throw VkException{"Could not create fence", result};
			}

			return UniqueVkFence{fence, VkFenceDestroyer{device_}};
		}()};
		VkFence       fence{mandatory_fence == VK_NULL_HANDLE ? unique_fence.get() : mandatory_fence};

		submit(fence, submit_info);

		vkWaitForFences(device_, 1, &fence, VK_TRUE, std::numeric_limits<std::size_t>::max());
	}

	VkCommandBuffer CommandBuffer::get() const {
		return cmd_buffer_.get();
	}
}// namespace raytracing::vulkan
