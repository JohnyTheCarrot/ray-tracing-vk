#include <iostream>
#define GLFW_INCLUDE_VULKAN

#include "diagnostics.h"
#include "src/vulkan/vkb_raii.h"

#include <GLFW/glfw3.h>
#include <VkBootstrap.h>
#include <format>
#include <vulkan/vulkan_core.h>

VkBool32 debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type,
        VkDebugUtilsMessengerCallbackDataEXT const *callback_data, void *user_data
) {
	auto const severity{vkb::to_string_message_severity(message_severity)};
	auto const type{vkb::to_string_message_type(message_type)};

	std::string completeMessage{std::format("[{}: {}] {}", severity, type, callback_data->pMessage)};
	raytracing::Logger::get_instance().Log(completeMessage);

	return VK_FALSE;
}

int main() {
	auto system_info_ret{vkb::SystemInfo::get_system_info()};
	if (!system_info_ret) {
		std::string message{std::format("Failed to get system info: {}", system_info_ret.error().message())};

		raytracing::Logger::get_instance().Log(raytracing::LogLevel::Error, std::move(message));
	}
	auto system_info{system_info_ret.value()};
	for (auto const &available_layer: system_info.available_layers) {
		std::cout << available_layer.layerName << " : " << available_layer.description << '\n';
	}


	vkb::InstanceBuilder builder{};
	auto const           instanceBuildResult = builder.set_app_name("Vulkan Ray Tracer")
	                                         .set_engine_name("Roingus Engine")
	                                         .request_validation_layers()
	                                         .set_debug_callback(debug_callback)
	                                         .build();

	if (!instanceBuildResult) {
		std::string message{std::format("Failed to build instance: {}", instanceBuildResult.error().message())};
		raytracing::Logger::get_instance().Log(raytracing::LogLevel::Error, std::move(message));

		return 1;
	}

	raytracing::UniqueVkbInstance uVkbInstance{instanceBuildResult.value()};
}
