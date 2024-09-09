#include "semaphore.h"
#include "src/diagnostics.h"
#include "src/vulkan/vk_exception.h"
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	VkSemaphoreDestroyer::VkSemaphoreDestroyer(VkDevice device)
	    : device_{device} {
	}

	void VkSemaphoreDestroyer::operator()(VkSemaphore semaphore) const {
		Logger::get_instance().log(LogLevel::Debug, "Destroying semaphore");
		vkDestroySemaphore(device_, semaphore, nullptr);
	}

	UniqueVkSemaphore create_semaphore(VkDevice device) {
		VkSemaphoreCreateInfo semaphore_info{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

		VkSemaphore semaphore{};
		if (VkResult const result{vkCreateSemaphore(device, &semaphore_info, nullptr, &semaphore)};
		    result != VK_SUCCESS) {
			throw VkException{"Could not create semaphore", result};
		}

		return UniqueVkSemaphore{semaphore, VkSemaphoreDestroyer{device}};
	}
}// namespace raytracing::vulkan
