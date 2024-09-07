#include "src/vulkan/surface.h"
#include "src/vulkan/vk_exception.h"
#include <stdexcept>
#define GLFW_INCLUDE_VULKAN

#include "src/window.h"

#include "diagnostics.h"
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

	std::string complete_message{std::format("[{}: {}] {}", severity, type, callback_data->pMessage)};
	raytracing::Logger::get_instance().log(complete_message);

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
	                                           .set_debug_callback(debug_callback)
	                                           .build();

	if (!instance_build_result) {
		std::string message{std::format("Failed to build instance: {}", instance_build_result.error().message())};
		Logger::get_instance().log(LogLevel::Error, std::move(message));

		return 1;
	}

	UniqueVkbInstance vkb_instance{instance_build_result.value()};

	glfwSetErrorCallback(glfw_error_callback);
	Window           window{vkb_instance.get(), 800, 600, "Vulkan Ray Tracer"};
	vulkan::Surface &surface{window.get_surface()};
	surface.select_physical_device();

	return 0;
}

int main() {
	/*auto system_info_ret{vkb::SystemInfo::get_system_info()};*/
	/*if (!system_info_ret) {*/
	/*	std::string message{std::format("Failed to get system info: {}", system_info_ret.error().message())};*/
	/**/
	/*	raytracing::Logger::get_instance().Log(raytracing::LogLevel::Error, std::move(message));*/
	/*}*/
	/*auto system_info{system_info_ret.value()};*/
	/*for (auto const &available_layer: system_info.available_layers) {*/
	/*	std::cout << available_layer.layerName << " : " << available_layer.description << '\n';*/
	/*}*/
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
	}};

	glfwTerminate();
	return exitCode();
}
