#define GLFW_INCLUDE_VULKAN
#include "diagnostics.h"
#include "vulkan/instance.h"
#include "vulkan/vk_exception.h"
#include "window.h"

#include <GLFW/glfw3.h>

int main() {
	if (glfwInit() == GLFW_FALSE) {
		raytracing::Logger::GetInstance().Log(raytracing::LogLevel::Error, "Failed to initialize GLFW");
		return 1;
	}

	constexpr std::string_view applicationName{"Raytracing"};
	constexpr std::string_view engineName{"Raytracing Engine"};

	try {
		raytracing::Window           window{800, 600, applicationName};
		raytracing::vulkan::Instance instance{applicationName, engineName};
		window.MainLoop();
	} catch (const raytracing::vulkan::VkException &ex) {
		raytracing::Logger::GetInstance().Log(raytracing::LogLevel::Error, ex.what());
	}

	glfwTerminate();
	return 0;
}
