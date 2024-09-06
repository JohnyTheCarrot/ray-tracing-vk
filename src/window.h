#ifndef WINDOW_H
#define WINDOW_H

#include "src/vulkan/surface.h"
#include <memory>
#include <string_view>

struct VkInstance_T;
using VkInstance = VkInstance_T *;

struct GLFWwindow;

namespace raytracing {
	namespace vulkan {
		class Surface;
	}

	class GLFWWindowDestroyer final {
	public:
		void operator()(GLFWwindow *window) const;
	};

	using UniqueGLFWWindow = std::unique_ptr<GLFWwindow, GLFWWindowDestroyer>;

	class Window final {
		UniqueGLFWWindow window_;
		vulkan::Surface  surface_;

	public:
		Window(VkInstance instance, int width, int height, std::string_view title);

		[[nodiscard]]
		GLFWwindow *get() const noexcept;

		void main_loop() const;

		[[nodiscard]]
		vulkan::Surface &get_surface() noexcept;

		[[nodiscard]]
		vulkan::Surface const &get_surface() const noexcept;
	};
}// namespace raytracing

#endif//WINDOW_H
