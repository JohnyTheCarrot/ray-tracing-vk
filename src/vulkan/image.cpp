#include "image.h"
#include "command_buffer.h"
#include "command_pool.h"
#include "src/diagnostics.h"
#include "src/vulkan/image_view.h"
#include "src/vulkan/vk_exception.h"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	VkImageDestroyer::VkImageDestroyer(VmaAllocator allocator, VmaAllocation allocation)
	    : allocator_{allocator}
	    , allocation_{allocation} {
	}

	void VkImageDestroyer::operator()(VkImage image) const {
		Logger::get_instance().log(LogLevel::Debug, "Destroying image");
		vmaDestroyImage(allocator_, image, allocation_);
	}

	VkSamplerDestroyer::VkSamplerDestroyer(VkDevice device)
	    : device_{device} {
	}

	void VkSamplerDestroyer::operator()(VkSampler sampler) const {
		Logger::get_instance().log(LogLevel::Debug, "Destroying sampler");
		vkDestroySampler(device_, sampler, nullptr);
	}

	Image::Image(UniqueVkImage &&image, std::uint32_t width, std::uint32_t height, VkDevice device, VkFormat format)
	    : image_{std::move(image)}
	    , format_{format}
	    , device_{device}
	    , extent_{width, height} {
	}

	VkImage Image::get() const {
		return image_.get();
	}

	UniqueVkImageView Image::create_image_view(VkImageAspectFlags aspect_flags) const {
		VkImageViewCreateInfo create_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
		create_info.format                          = format_;
		create_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
		create_info.image                           = image_.get();
		create_info.subresourceRange.aspectMask     = aspect_flags;
		create_info.subresourceRange.baseMipLevel   = 0;
		create_info.subresourceRange.levelCount     = VK_REMAINING_MIP_LEVELS;
		create_info.subresourceRange.baseArrayLayer = 0;
		create_info.subresourceRange.layerCount     = VK_REMAINING_ARRAY_LAYERS;

		VkImageView view{};
		if (VkResult const result{vkCreateImageView(device_, &create_info, VK_NULL_HANDLE, &view)};
		    result != VK_SUCCESS) {
			throw VkException{"Failed to create image view", result};
		}

		return UniqueVkImageView{view, VkImageViewDestroyer{device_}};
	}

	UniqueVkSampler Image::create_sampler() const {
		VkSamplerCreateInfo create_info{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
		create_info.magFilter               = VK_FILTER_NEAREST;
		create_info.minFilter               = VK_FILTER_NEAREST;
		create_info.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		create_info.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		create_info.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		create_info.anisotropyEnable        = VK_FALSE;
		create_info.maxAnisotropy           = 1.f;
		create_info.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		create_info.unnormalizedCoordinates = VK_FALSE;
		create_info.compareEnable           = VK_FALSE;
		create_info.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		VkSampler sampler{};
		if (VkResult const result{vkCreateSampler(device_, &create_info, nullptr, &sampler)}; result != VK_SUCCESS) {
			throw VkException{"Failed to create sampler", result};
		}

		return UniqueVkSampler{sampler, VkSamplerDestroyer{device_}};
	}

	VkExtent2D Image::get_extent() const {
		return extent_;
	}

	void Image::transition_layout(
	        CommandPool const &command_pool, VkImageLayout old_layout, VkImageLayout new_layout,
	        VkImageAspectFlags aspect_flags
	) const {
		VkImageSubresourceRange subresource_range;
		subresource_range.aspectMask     = aspect_flags;
		subresource_range.levelCount     = VK_REMAINING_MIP_LEVELS;
		subresource_range.layerCount     = VK_REMAINING_ARRAY_LAYERS;
		subresource_range.baseMipLevel   = 0;
		subresource_range.baseArrayLayer = 0;

		VkImageMemoryBarrier image_memory_barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
		image_memory_barrier.oldLayout        = old_layout;
		image_memory_barrier.newLayout        = new_layout;
		image_memory_barrier.image            = image_.get();
		image_memory_barrier.subresourceRange = subresource_range;

		VkPipelineStageFlags source_stage;
		VkPipelineStageFlags destination_stage;
		if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			image_memory_barrier.srcAccessMask = 0;
			image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			source_stage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		} else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			source_stage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		} else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		auto const &command_buffer{command_pool.allocate_command_buffer()};
		command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		vkCmdPipelineBarrier(
		        command_buffer.get(), source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1,
		        &image_memory_barrier
		);
		command_buffer.end();
		command_buffer.submit_and_wait(VK_NULL_HANDLE);
	}
}// namespace raytracing::vulkan
