#include "src/vulkan/command_buffer.h"
#include "src/vulkan/engine.h"
#include <filesystem>
#include <fstream>
#include <stdexcept>
#define GLFW_INCLUDE_VULKAN


#include "diagnostics.h"
#include "scene.h"

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

std::vector<char> read_file(std::filesystem::path const &path) {
	std::ifstream file{path, std::ios::binary | std::ios::ate};

	if (!file) {
		throw std::runtime_error{std::string{"couldn't open file " + path.string()}};
	}

	size_t            filesize = (size_t) file.tellg();
	std::vector<char> buffer(filesize);
	file.seekg(0);
	file.read(buffer.data(), filesize);

	file.close();
	return buffer;
}

int run() {
	using namespace raytracing;
	vulkan::Engine engine{"Vulkan Raytracer"};

	Logger::get_instance().log(LogLevel::Debug, "vulkan ready");

	Scene scene{
	        engine.get_logical_device(), engine.get_command_pool(), engine.get_allocator(), "resources/maps/p2-map.glb",
	        GltfScene{}
	};

	Logger::get_instance().log(LogLevel::Debug, "gltf done");

	auto vert_shader_bytecode{read_file("shaders/shader.vert.spv")};
	auto frag_shader_bytecode{read_file("shaders/shader.frag.spv")};

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
		}

		return 1;
	}()};

	glfwTerminate();
	return exitCode;
}
