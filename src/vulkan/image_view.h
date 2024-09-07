#ifndef SRC_VULKAN_IMAGE_VIEW_H_
#define SRC_VULKAN_IMAGE_VIEW_H_

#include <memory>

struct VkDevice_T;
using VkDevice = VkDevice_T *;

struct VkImageView_T;
using VkImageView = VkImageView_T *;

namespace raytracing::vulkan {
	class ImageViewDestroyer final {
		VkDevice device_;

	public:
		explicit ImageViewDestroyer(VkDevice device);

		void operator()(VkImageView imageView);
	};

	using UniqueImageView = std::unique_ptr<VkImageView_T, ImageViewDestroyer>;
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_IMAGE_VIEW_H_
