#ifndef SRC_VULKAN_COMMAND_BUFFER_H_
#define SRC_VULKAN_COMMAND_BUFFER_H_

#include <memory>
#include <vulkan/vulkan_core.h>
struct VkCommandBuffer_T;
using VkCommandBuffer = VkCommandBuffer_T *;

struct VkCommandPool_T;
using VkCommandPool = VkCommandPool_T *;

struct VkDevice_T;
using VkDevice = VkDevice_T *;

namespace raytracing::vulkan {
	class CommandBufferDestroyer final {
		VkDevice      device_;
		VkCommandPool cmd_pool_;

	public:
		CommandBufferDestroyer(VkDevice device, VkCommandPool cmd_pool);

		void operator()(VkCommandBuffer buffer);
	};

	using UniqueVkCommandBuffer = std::unique_ptr<VkCommandBuffer_T, CommandBufferDestroyer>;

	class CommandBuffer final {
		UniqueVkCommandBuffer cmd_buffer_;
		VkQueue               queue_;
		VkDevice              device_;

	public:
		CommandBuffer(UniqueVkCommandBuffer &&buffer, VkDevice device, VkQueue queue);

		void begin(VkCommandBufferUsageFlags flags) const;

		void end() const;

		void reset() const;

		void submit(VkFence optional_fence, VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO}) const;

		void submit_and_wait(VkFence optional_fence, VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO}) const;

		[[nodiscard]]
		VkCommandBuffer get() const;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_COMMAND_BUFFER_H_
