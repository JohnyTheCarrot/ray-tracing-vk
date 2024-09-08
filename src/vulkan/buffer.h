#ifndef SRC_VULKAN_BUFFER_H_
#define SRC_VULKAN_BUFFER_H_

#include <span>
#include <vulkan/vulkan_core.h>

#include "vk_exception.h"
#include "vkb_raii.h"

namespace raytracing::vulkan {
	class CommandBuffer;

	class BufferDestroyer final {
		VmaAllocator  allocator_;
		VmaAllocation allocation_;

	public:
		BufferDestroyer(VmaAllocator allocator, VmaAllocation allocation);

		void operator()(VkBuffer buffer);
	};

	using UniqueVkBuffer = std::unique_ptr<VkBuffer_T, BufferDestroyer>;

	class Buffer final {
		UniqueVkBuffer buffer_;
		VmaAllocation  allocation_;
		VkDeviceSize   size_;
		VkDevice       device_;

	public:
		Buffer(VkDevice device, VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage_flags,
		       VmaAllocationCreateFlags alloc_flags);

		template<class V>
		Buffer(VkDevice device, VmaAllocator allocator, std::span<V> span, VkBufferUsageFlags usage_flags)
		    : Buffer{device, allocator, span.size_bytes(), usage_flags,
		             VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT} {
			if (VkResult const result{
			            vmaCopyMemoryToAllocation(allocator, span.data(), allocation_, 0, span.size_bytes())
			    };
			    result != VK_SUCCESS) {
				throw vulkan::VkException{"Failed to copy index data to index buffer", result};
			}
		}

		[[nodiscard]]
		VkBuffer get() const noexcept;

		[[nodiscard]]
		VkDeviceAddress get_device_address() const;

		[[nodiscard]]
		VkDeviceSize get_size() const noexcept;

		void copy_to(CommandBuffer const &command_buffer, Buffer &buffer) const;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_BUFFER_H_
