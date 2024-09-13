#ifndef SRC_VULKAN_BUFFER_H_
#define SRC_VULKAN_BUFFER_H_

#include <glm/glm.hpp>
#include <optional>
#include <span>
#include <vulkan/vulkan_core.h>

#include "vk_exception.h"
#include "vkb_raii.h"

namespace raytracing::vulkan {
	class CommandPool;

	class Image;

	class BufferDestroyer final {
		VmaAllocator  allocator_;
		VmaAllocation allocation_;

	public:
		BufferDestroyer(VmaAllocator allocator, VmaAllocation allocation);

		void operator()(VkBuffer buffer) const;
	};

	using UniqueVkBuffer = std::unique_ptr<VkBuffer_T, BufferDestroyer>;

	class Buffer;

	class Allocator;

	class MappedBufferPtr final {
		Buffer const *buff_;
		void         *mapped_ptr_;

	public:
		explicit MappedBufferPtr(void *mapped_ptr, Buffer const &buff);

		~MappedBufferPtr();

		MappedBufferPtr(MappedBufferPtr &&);

		MappedBufferPtr &operator=(MappedBufferPtr &&);

		MappedBufferPtr(MappedBufferPtr const &) = delete;

		MappedBufferPtr &operator=(MappedBufferPtr const &) = delete;

		[[nodiscard]]
		void *get_mapped_ptr() const;
	};

	class Buffer final {
		UniqueVkBuffer buffer_;
		VmaAllocator   allocator_;
		VmaAllocation  allocation_;
		VkDeviceSize   size_;
		VkDevice       device_;

	public:
		Buffer(VkDevice device, VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage_flags,
		       VmaAllocationCreateFlags alloc_flags, VkMemoryPropertyFlags required_memory_flags = 0,
		       std::optional<VkDeviceSize> alignment = std::nullopt);

		template<class V>
		Buffer(VkDevice device, VmaAllocator allocator, std::span<V> span, VkBufferUsageFlags usage_flags,
		       VmaAllocationCreateFlags alloc_flags        = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		       VkMemoryPropertyFlags required_memory_flags = 0, std::optional<VkDeviceSize> alignment = std::nullopt)
		    : Buffer{device, allocator, span.size_bytes(), usage_flags, alloc_flags, required_memory_flags, alignment} {
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

		void copy_to(CommandPool const &command_pool, Buffer const &buffer) const;

		void copy_to(CommandPool const &command_pool, Image const &image) const;

		[[nodiscard]]
		MappedBufferPtr map_memory() const;

		void unmap_memory() const;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_BUFFER_H_
