#ifndef SRC_VULKAN_FRAME_BUFFER_H_
#define SRC_VULKAN_FRAME_BUFFER_H_

#include <memory>

struct VkDevice_T;
using VkDevice = VkDevice_T *;

struct VkFramebuffer_T;
using VkFramebuffer = VkFramebuffer_T *;

namespace raytracing::vulkan {
	class VkFramebufferDestroyer final {
		VkDevice device_;

	public:
		explicit VkFramebufferDestroyer(VkDevice device);

		void operator()(VkFramebuffer framebuffer) const;
	};

	using UniqueVkFramebuffer = std::unique_ptr<VkFramebuffer_T, VkFramebufferDestroyer>;
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_FRAME_BUFFER_H_
