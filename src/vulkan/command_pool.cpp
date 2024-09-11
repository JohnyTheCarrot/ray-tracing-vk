#include "command_pool.h"
#include "VkBootstrap.h"
#include "logical_device.h"
#include "src/diagnostics.h"
#include "src/vulkan/command_buffer.h"
#include "src/vulkan/vk_exception.h"
#include <algorithm>
#include <iterator>
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	CommandPoolDestroyer::CommandPoolDestroyer(VkDevice device)
	    : device_{device} {
	}

	void CommandPoolDestroyer::operator()(VkCommandPool command_pool) {
		Logger::get_instance().log(LogLevel::Debug, "Destroying command pool");
		vkDestroyCommandPool(device_, command_pool, nullptr);
	}

	CommandPool::CommandPool(vkb::QueueType queue_type, LogicalDevice const &device, VkCommandPoolCreateFlags flags)
	    : command_pool_{[&] {
		    VkCommandPoolCreateInfo pool_info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
		    pool_info.flags            = flags;
		    pool_info.queueFamilyIndex = device.get_queue_index(queue_type);

		    VkCommandPool command_pool{};
		    if (VkResult const result{vkCreateCommandPool(device.get().device, &pool_info, nullptr, &command_pool)};
		        result != VK_SUCCESS) {
			    throw VkException{"Failed to create command pool", result};
		    }

		    return UniqueVkCommandPool{command_pool, CommandPoolDestroyer{device.get().device}};
	    }()}
	    , device_{&device}
	    , queue_{device.get_queue(queue_type)} {
	}

	std::vector<CommandBuffer> CommandPool::allocate_command_buffers(std::size_t num) const {
		VkCommandBufferAllocateInfo alloc_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
		alloc_info.commandPool        = command_pool_.get();
		alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = num;

		std::vector<VkCommandBuffer> raw_buffers(num);
		if (VkResult const result{vkAllocateCommandBuffers(device_->get(), &alloc_info, raw_buffers.data())};
		    result != VK_SUCCESS) {
			throw VkException{"Could not allocate command buffers", result};
		}

		std::vector<CommandBuffer> buffers;
		buffers.reserve(num);

		std::transform(
		        raw_buffers.cbegin(), raw_buffers.cend(), std::back_inserter(buffers),
		        [&](VkCommandBuffer buff) {
			        return CommandBuffer{
			                UniqueVkCommandBuffer{
			                        buff, CommandBufferDestroyer{device_->get().device, command_pool_.get()}
			                },
			                device_->get().device, queue_
			        };
		        }
		);

		return buffers;
	}

	CommandBuffer CommandPool::allocate_command_buffer() const {
		auto buffers{allocate_command_buffers(1)};
		return std::move(buffers.at(0));
	}
}// namespace raytracing::vulkan
