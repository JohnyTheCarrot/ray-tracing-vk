#include "src/vulkan/engine.h"
#define GLFW_INCLUDE_VULKAN


#include "diagnostics.h"

#include <VkBootstrap.h>
#include <format>

#include <GLFW/glfw3.h>

VkBool32 debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type,
        VkDebugUtilsMessengerCallbackDataEXT const *callback_data, void *user_data
) {
	auto const severity{vkb::to_string_message_severity(message_severity)};
	auto const type{vkb::to_string_message_type(message_type)};
	auto const is_error{message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT};

	std::string complete_message{std::format("[{}: {}] {}", severity, type, callback_data->pMessage)};

	auto &logger_instance{raytracing::Logger::get_instance()};
	if (is_error) {
		logger_instance.error(complete_message);
	} else {
		logger_instance.log(complete_message);
	}

	return VK_FALSE;
}

void glfw_error_callback(int error, char const *description) {
	raytracing::Logger::get_instance().log(raytracing::LogLevel::Error, description);
}

int run() {
	using namespace raytracing;
	vulkan::Engine engine{"Vulkan Raytracer"};

	Logger::get_instance().log(LogLevel::Debug, "vulkan ready");

	Logger::get_instance().log(LogLevel::Debug, "gltf done");

	engine.main_loop();

	Logger::get_instance().log(LogLevel::Debug, "done");

	return 0;
}

int main() {
	auto const exitCode{[] {
		try {
			return run();
		} catch (std::exception const &ex) {
			std::string message{std::format("Exiting with error: {}", ex.what())};
			raytracing::Logger::get_instance().log(raytracing::LogLevel::Error, std::move(message));
		} catch (...) { raytracing::Logger::get_instance().log(raytracing::LogLevel::Error, "Unknown error"); }

		return 1;
	}()};

	glfwTerminate();
	return exitCode;
}
