#ifndef SRC_VULKAN_SEMAPHORE_H_
#define SRC_VULKAN_SEMAPHORE_H_

#include <memory>

struct VkDevice_T;
using VkDevice = VkDevice_T *;

struct VkSemaphore_T;
using VkSemaphore = VkSemaphore_T *;

namespace raytracing::vulkan {
	class VkSemaphoreDestroyer final {
		VkDevice device_;

	public:
		explicit VkSemaphoreDestroyer(VkDevice device);

		void operator()(VkSemaphore semaphore) const;
	};

	using UniqueVkSemaphore = std::unique_ptr<VkSemaphore_T, VkSemaphoreDestroyer>;
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_SEMAPHORE_H_
