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

	class VkSamplerDestroyer final {
		VkDevice device_;

	public:
		explicit VkSamplerDestroyer(VkDevice device);

		void operator()(VkSampler sampler) const;
	};

	using UniqueVkSampler = std::unique_ptr<VkSampler_T, VkSamplerDestroyer>;

	class CommandPool;

	class Image final {
		UniqueVkImage image_;
		VkDevice      device_;
		VkFormat      format_;
		VkExtent2D    extent_;

	public:
		Image(UniqueVkImage &&image, std::uint32_t width, std::uint32_t height, VkDevice device, VkFormat format);

		[[nodiscard]]
		VkImage get() const;

		[[nodiscard]]
		UniqueVkImageView create_image_view(VkImageAspectFlags aspect_flags) const;

		[[nodiscard]]
		UniqueVkSampler create_sampler() const;

		[[nodiscard]]
		VkExtent2D get_extent() const;

		void transition_layout(
		        CommandPool const &command_pool, VkImageLayout old_layout, VkImageLayout new_layout,
		        VkImageAspectFlags aspect_flags
		) const;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_IMAGE_H_
