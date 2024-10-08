#include "buffer.h"
#include "command_buffer.h"
#include "command_pool.h"
#include "image.h"
#include "src/vulkan/vk_exception.h"
#include <optional>
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	BufferDestroyer::BufferDestroyer(VmaAllocator allocator, VmaAllocation allocation)
	    : allocator_{allocator}
	    , allocation_{allocation} {
	}

	void BufferDestroyer::operator()(VkBuffer buffer) const {
		/*Logger::get_instance().log(LogLevel::Debug, "Destroying buffer");*/
		vmaDestroyBuffer(allocator_, buffer, allocation_);
	}

	MappedBufferPtr::MappedBufferPtr(void *mapped_ptr, Buffer const &buff)
	    : buff_{&buff}
	    , mapped_ptr_{mapped_ptr} {
	}

	MappedBufferPtr::~MappedBufferPtr() {
		if (mapped_ptr_ != nullptr)
			buff_->unmap_memory();
	}

	MappedBufferPtr::MappedBufferPtr(MappedBufferPtr &&other)
	    : buff_{other.buff_}
	    , mapped_ptr_{other.mapped_ptr_} {
		other.mapped_ptr_ = nullptr;
	}

	MappedBufferPtr &MappedBufferPtr::operator=(MappedBufferPtr &&other) {
		if (&other == this)
			return *this;

		buff_             = other.buff_;
		mapped_ptr_       = other.mapped_ptr_;
		other.mapped_ptr_ = nullptr;

		return *this;
	}

	void *MappedBufferPtr::get_mapped_ptr() const {
		return mapped_ptr_;
	}

	Buffer::Buffer(
	        VkDevice device, VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage_flags,
	        VmaAllocationCreateFlags alloc_flags, VkMemoryPropertyFlags required_memory_flags,
	        std::optional<VkDeviceSize> alignment
	)
	    : buffer_{[&] {
		    VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		    buffer_info.size  = size;
		    buffer_info.usage = usage_flags;

		    VmaAllocationCreateInfo alloc_info{};
		    alloc_info.usage         = VMA_MEMORY_USAGE_AUTO;
		    alloc_info.flags         = alloc_flags;
		    alloc_info.requiredFlags = required_memory_flags;

		    VkBuffer buffer{};

		    if (alignment.has_value()) {
			    if (VkResult const result{vmaCreateBufferWithAlignment(
			                allocator, &buffer_info, &alloc_info, alignment.value(), &buffer, &allocation_, nullptr
			        )};
			        result != VK_SUCCESS) {
				    throw vulkan::VkException{"Failed to create buffer with alignment", result};
			    }
		    } else {
			    if (VkResult const result{
			                vmaCreateBuffer(allocator, &buffer_info, &alloc_info, &buffer, &allocation_, nullptr)
			        };
			        result != VK_SUCCESS) {
				    throw vulkan::VkException{"Failed to create buffer", result};
			    }
		    }

		    return vulkan::UniqueVkBuffer{buffer, vulkan::BufferDestroyer{allocator, allocation_}};
	    }()}
	    , allocator_{allocator}
	    , size_{size}
	    , device_{device} {
	}

	VkBuffer Buffer::get() const noexcept {
		return buffer_.get();
	}

	void Buffer::copy_to(CommandPool const &command_pool, Buffer const &buffer) const {
		VkBufferCopy copy_region{};
		copy_region.srcOffset = 0;
		copy_region.dstOffset = 0;
		copy_region.size      = size_;

		auto const command_buffer{command_pool.allocate_command_buffer()};
		command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		vkCmdCopyBuffer(command_buffer.get(), buffer_.get(), buffer.get(), 1, &copy_region);
		command_buffer.end();
		command_buffer.submit_and_wait(VK_NULL_HANDLE);
	}

	void Buffer::copy_to(CommandPool const &command_pool, Image const &image) const {
		VkBufferImageCopy copy_region{};
		copy_region.bufferOffset                    = 0;
		copy_region.bufferRowLength                 = 0;
		copy_region.bufferImageHeight               = 0;
		copy_region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		copy_region.imageSubresource.mipLevel       = 0;
		copy_region.imageSubresource.baseArrayLayer = 0;
		copy_region.imageSubresource.layerCount     = 1;
		copy_region.imageOffset                     = {0, 0};
		copy_region.imageExtent                     = {image.get_extent().width, image.get_extent().height, 1};

		auto const command_buffer{command_pool.allocate_command_buffer()};
		command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		vkCmdCopyBufferToImage(
		        command_buffer.get(), buffer_.get(), image.get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region
		);
		command_buffer.end();
		command_buffer.submit_and_wait(VK_NULL_HANDLE);
	}

	VkDeviceAddress Buffer::get_device_address() const {
		VkBufferDeviceAddressInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, buffer_.get()};
		return vkGetBufferDeviceAddress(device_, &bufferInfo);
	}

	VkDeviceSize Buffer::get_size() const noexcept {
		return size_;
	}

	MappedBufferPtr Buffer::map_memory() const {
		void *map{};

		if (VkResult const result{vmaMapMemory(allocator_, allocation_, &map)}; result != VK_SUCCESS) {
			throw VkException{"Failed to map memory", result};
		}

		return MappedBufferPtr{map, *this};
	}

	void Buffer::unmap_memory() const {
		vmaUnmapMemory(allocator_, allocation_);
	}
}// namespace raytracing::vulkan
