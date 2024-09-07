#include <stdexcept>
#define GLFW_INCLUDE_VULKAN

#include "src/window.h"

#include "diagnostics.h"
#include "src/vulkan/surface.h"
#include "src/vulkan/swapchain.h"
#include "src/vulkan/vk_exception.h"
#include "src/vulkan/vkb_raii.h"

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
	vkb::InstanceBuilder builder{};
	auto const           instance_build_result = builder.set_app_name("Vulkan Ray Tracer")
	                                           .set_engine_name("Roingus Engine")
	                                           .request_validation_layers()
	                                           .require_api_version(1, 1, 0)
	                                           .set_debug_callback(debug_callback)
	                                           .build();

	if (!instance_build_result) {
		std::string message{std::format("Failed to build instance: {}", instance_build_result.error().message())};
		Logger::get_instance().log(LogLevel::Error, std::move(message));

		return 1;
	}

	UniqueVkbInstance vkb_instance{instance_build_result.value()};

	glfwSetErrorCallback(glfw_error_callback);
	Window            window{vkb_instance.get(), 800, 600, "Vulkan Ray Tracer"};
	vulkan::Surface  &surface{window.get_surface()};
	auto const        physical_device{surface.select_physical_device()};
	auto const        logical_device{physical_device.create_logical_device()};
	vulkan::Swapchain swapchain{logical_device};
	swapchain.recreate();

	window.main_loop();

	Logger::get_instance().log(LogLevel::Error, "done");

	return 0;
}

int main() {
	auto const exitCode{[] {
		try {
			return run();
		} catch (raytracing::vulkan::VkException const &ex) {
			raytracing::Logger::get_instance().log(raytracing::LogLevel::Error, ex.what());
		} catch (std::runtime_error const &ex) {
			raytracing::Logger::get_instance().log(raytracing::LogLevel::Error, ex.what());
		} catch (std::exception const &ex) {
			raytracing::Logger::get_instance().log(raytracing::LogLevel::Error, ex.what());
		}

		return 1;
	}()};

	glfwTerminate();
	return exitCode;
}
