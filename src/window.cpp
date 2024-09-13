#include "window.h"
#include "VkBootstrap.h"
#include "src/camera.h"
#include <chrono>

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

	Window::Window(vkb::Instance &instance, int width, int height, std::string_view title)
	    : window_{[width, height, title]() {
		    if (!glfwInit()) {
			    throw std::runtime_error{"Failed to init GLFW"};
		    }

		    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		    return glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
	    }()}
	    , surface_{[this, &instance] {
		    VkSurfaceKHR surface{};
		    if (VkResult result{glfwCreateWindowSurface(instance, window_.get(), nullptr, &surface)};
		        result != VK_SUCCESS) {
			    throw vulkan::VkException{"Failed to create window surface", result};
		    }

		    return vulkan::Surface{instance, surface};
	    }()} {
		if (window_ == nullptr) {
			throw std::runtime_error{"Could not create window"};
		}
		glfwSetInputMode(window_.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		Logger::get_instance().log(LogLevel::Debug, "Created window and its surface");
	}

	bool Window::get_should_close() const {
		return glfwWindowShouldClose(window_.get());
	}

	void Window::poll_events() const {
		glfwPollEvents();

		constexpr float speed{150.f};
		constexpr float rotate_speed{10.0f};
		static auto     startTime = std::chrono::high_resolution_clock::now();

		auto  currentTime = std::chrono::high_resolution_clock::now();
		float time        = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		startTime         = currentTime;

		Camera &cam{Camera::get_instance()};
		if (glfwGetKey(window_.get(), GLFW_KEY_W)) {
			cam.translate(glm::vec3{0.f, 0.f, 1.f} * speed * time);
		}
		if (glfwGetKey(window_.get(), GLFW_KEY_SPACE)) {
			cam.translate(glm::vec3{0.f, -1.f, 0.f} * speed * time);
		}
		if (glfwGetKey(window_.get(), GLFW_KEY_C)) {
			cam.translate(glm::vec3{0.f, 1.f, 0.f} * speed * time);
		}
		if (glfwGetKey(window_.get(), GLFW_KEY_W)) {
			cam.translate(glm::vec3{0.f, 0.f, 1.f} * speed * time);
		}
		if (glfwGetKey(window_.get(), GLFW_KEY_S)) {
			cam.translate(glm::vec3{0.f, 0.f, -1.f} * speed * time);
		}
		if (glfwGetKey(window_.get(), GLFW_KEY_A)) {
			cam.translate(glm::vec3{1.f, 0.f, 0.f} * speed * time);
		}
		if (glfwGetKey(window_.get(), GLFW_KEY_D)) {
			cam.translate(glm::vec3{-1.f, 0.f, 0.f} * speed * time);
		}

		double x_pos{};
		double y_pos{};
		glfwGetCursorPos(window_.get(), &x_pos, &y_pos);

		if (x_pos != 0.f || y_pos != 0.f)
			cam.rotate(x_pos * rotate_speed * time, y_pos * rotate_speed * time);
		glfwSetCursorPos(window_.get(), 0, 0);
	}

	vulkan::Surface &Window::get_surface() noexcept {
		return surface_;
	}

	vulkan::Surface const &Window::get_surface() const noexcept {
		return surface_;
	}
}// namespace raytracing
