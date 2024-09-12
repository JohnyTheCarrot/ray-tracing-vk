#ifndef SRC_VULKAN_IMAGE_H_
#define SRC_VULKAN_IMAGE_H_

#include "src/vulkan/image_view.h"
#include <memory>
#include <vulkan/vulkan_core.h>

struct VmaAllocator_T;
using VmaAllocator = VmaAllocator_T *;

struct VmaAllocation_T;
using VmaAllocation = VmaAllocation_T *;

namespace raytracing::vulkan {
	class VkImageDestroyer final {
		VmaAllocator  allocator_;
		VmaAllocation allocation_;

	public:
		VkImageDestroyer(VmaAllocator allocator, VmaAllocation allocation);

		void operator()(VkImage image) const;
	};

	using UniqueVkImage = std::unique_ptr<VkImage_T, VkImageDestroyer>;

	class Image final {
		UniqueVkImage image_;
		VkDevice      device_;
		VkFormat      format_;

	public:
		Image(UniqueVkImage &&image, VkDevice device, VkFormat format);

		[[nodiscard]]
		VkImage get() const;

		[[nodiscard]]
		UniqueVkImageView create_image_view(VkImageAspectFlags aspect_flags) const;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_IMAGE_H_
