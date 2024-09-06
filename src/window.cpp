#include "window.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "src/diagnostics.h"
#include "src/vulkan/surface.h"
#include "src/vulkan/vk_exception.h"
#include <stdexcept>

namespace raytracing {
	void GLFWWindowDestroyer::operator()(GLFWwindow *window) const {
		Logger::get_instance().log(LogLevel::Debug, "Destroying window");
		glfwDestroyWindow(window);
	}

	Window::Window(VkInstance instance, int width, int height, std::string_view title)
	    : window_{[width, height, title]() {
		    Logger::get_instance().log(LogLevel::Debug, "Creating window");
		    if (!glfwInit()) {
			    throw std::runtime_error{"Failed to init GLFW"};
		    }
		    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		    return glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
	    }()}
	    , surface_{[this, instance] {
		    VkSurfaceKHR surface{};
		    if (VkResult result{glfwCreateWindowSurface(instance, get(), nullptr, &surface)}; result != VK_SUCCESS) {
			    throw vulkan::VkException{"Failed to create window surface", result};
		    }

		    return vulkan::Surface{instance, surface};
	    }()} {
		if (window_ == nullptr) {
			throw std::runtime_error{"Could not create window"};
		}

		Logger::get_instance().log(LogLevel::Debug, "Created window");
	}

	GLFWwindow *Window::get() const noexcept {
		return window_.get();
	}

	void Window::main_loop() const {
		while (!glfwWindowShouldClose(window_.get())) { glfwPollEvents(); }
	}

	vulkan::Surface &Window::get_surface() noexcept {
		return surface_;
	}

	vulkan::Surface const &Window::get_surface() const noexcept {
		return surface_;
	}
}// namespace raytracing
