#include "window.h"

#include "diagnostics.h"

#include <GLFW/glfw3.h>

namespace raytracing {
	void GLFWWindowDestroyer::operator()(GLFWwindow *window) const {
		Logger::GetInstance().Log(LogLevel::Debug, "Destroying window");
		glfwDestroyWindow(window);
	}

	Window::Window(int width, int height, std::string_view title)
	    : m_Window{[width, height, title]() {
		    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		    return glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
	    }()} {
		Logger::GetInstance().Log(LogLevel::Debug, "Creating window");
	}

	void Window::MainLoop() const {
		while (!glfwWindowShouldClose(m_Window.get())) { glfwPollEvents(); }
	}
}// namespace raytracing
