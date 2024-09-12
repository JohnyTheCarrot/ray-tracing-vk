#include "image.h"
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

	Image::Image(UniqueVkImage &&image, VkDevice device, VkFormat format)
	    : image_{std::move(image)}
	    , format_{format}
	    , device_{device} {
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
}// namespace raytracing::vulkan
