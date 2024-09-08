#include "buffer.h"
#include "command_buffer.h"
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	BufferDestroyer::BufferDestroyer(VmaAllocator allocator, VmaAllocation allocation)
	    : allocator_{allocator}
	    , allocation_{allocation} {
	}

	void BufferDestroyer::operator()(VkBuffer buffer) {
		/*Logger::get_instance().log(LogLevel::Debug, "Destroying VkBuffer");*/
		vmaDestroyBuffer(allocator_, buffer, allocation_);
	}

	Buffer::Buffer(
	        VkDevice device, VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage_flags,
	        VmaAllocationCreateFlags alloc_flags
	)
	    : buffer_{[&] {
		    VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		    buffer_info.size  = size;
		    buffer_info.usage = usage_flags;

		    VmaAllocationCreateInfo alloc_info{};
		    alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
		    alloc_info.flags = alloc_flags;

		    VkBuffer buffer{};

		    if (VkResult const result{
		                vmaCreateBuffer(allocator, &buffer_info, &alloc_info, &buffer, &allocation_, nullptr)
		        };
		        result != VK_SUCCESS) {
			    throw vulkan::VkException{"Failed to create buffer", result};
		    }

		    return vulkan::UniqueVkBuffer{buffer, vulkan::BufferDestroyer{allocator, allocation_}};
	    }()}
	    , size_{size}
	    , device_{device} {
	}

	VkBuffer Buffer::get() const noexcept {
		return buffer_.get();
	}

	void Buffer::copy_to(CommandBuffer const &command_buffer, Buffer &buffer) const {
		VkBufferCopy copy_region{};
		copy_region.srcOffset = 0;
		copy_region.dstOffset = 0;
		copy_region.size      = size_;

		vkCmdCopyBuffer(command_buffer.get(), buffer_.get(), buffer.get(), 1, &copy_region);
	}

	VkDeviceAddress Buffer::get_device_address() const {
		VkBufferDeviceAddressInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, buffer_.get()};
		return vkGetBufferDeviceAddress(device_, &bufferInfo);
	}

	VkDeviceSize Buffer::get_size() const noexcept {
		return size_;
	}
}// namespace raytracing::vulkan
