#ifndef WINDOW_H
#define WINDOW_H
#include <GLFW/glfw3.h>
#include <memory>
#include <string_view>

namespace raytracing {
	class GLFWWindowDestroyer final {
	public:
		void operator()(GLFWwindow *window) const;
	};

	using UniqueGLFWWindow = std::unique_ptr<GLFWwindow, GLFWWindowDestroyer>;

	class Window final {
		UniqueGLFWWindow m_Window;

	public:
		Window(int width, int height, std::string_view title);

		void MainLoop() const;
	};
}// namespace raytracing

#endif//WINDOW_H
