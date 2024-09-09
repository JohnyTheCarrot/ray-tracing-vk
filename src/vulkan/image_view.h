#ifndef SRC_VULKAN_IMAGE_VIEW_H_
#define SRC_VULKAN_IMAGE_VIEW_H_

#include <memory>

struct VkDevice_T;
using VkDevice = VkDevice_T *;

struct VkImageView_T;
using VkImageView = VkImageView_T *;

namespace raytracing::vulkan {
	class Swapchain;

	class VkImageViewDestroyer final {
		VkDevice device_;

	public:
		explicit VkImageViewDestroyer(VkDevice device);

		void operator()(VkImageView imageView) const;
	};

	using UniqueVkImageView = std::unique_ptr<VkImageView_T, VkImageViewDestroyer>;
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_IMAGE_VIEW_H_
