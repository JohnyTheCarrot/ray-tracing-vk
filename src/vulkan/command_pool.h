#ifndef SRC_VULKAN_COMMAND_POOL_H_
#define SRC_VULKAN_COMMAND_POOL_H_

#include "src/vulkan/command_buffer.h"
#include <cstdint>
#include <memory>
#include <vector>

struct VkDevice_T;
using VkDevice = VkDevice_T *;

struct VkCommandPool_T;
using VkCommandPool = VkCommandPool_T *;

namespace raytracing::vulkan {
	class LogicalDevice;

	class CommandPoolDestroyer final {
		VkDevice device_;

	public:
		explicit CommandPoolDestroyer(VkDevice device);

		void operator()(VkCommandPool command_pool);
	};

	using UniqueVkCommandPool = std::unique_ptr<VkCommandPool_T, CommandPoolDestroyer>;

	class CommandPool final {
		UniqueVkCommandPool  command_pool_;
		LogicalDevice const *device_;
		VkQueue              queue_;

	public:
		CommandPool(
		        std::uint32_t queue_family_idx, LogicalDevice const &device,
		        VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
		);

		[[nodiscard]]
		std::vector<CommandBuffer> allocate_command_buffers(std::size_t num) const;

		[[nodiscard]]
		CommandBuffer allocate_command_buffer() const;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_COMMAND_POOL_H_
