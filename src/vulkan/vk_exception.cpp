#include "vk_exception.h"
#include "src/diagnostics.h"

#include <format>
#include <vulkan/vk_enum_string_helper.h>

namespace raytracing::vulkan {
	VkException::VkException(std::string_view message, VkResult result)
	    : std::runtime_error{std::format("{}: {}", message, string_VkResult(result))} {
		Logger::get_instance().log(LogLevel::Error, what());
	}
}// namespace raytracing::vulkan
