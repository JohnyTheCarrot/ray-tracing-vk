#ifndef VK_EXCEPTION_H
#define VK_EXCEPTION_H

#include <stdexcept>
#include <string>
#include <string_view>
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	class VkException final : public std::runtime_error {
		std::string m_Message;

	public:
		VkException(std::string_view message, VkResult result);
	};
}// namespace raytracing::vulkan

#endif//VK_EXCEPTION_H
