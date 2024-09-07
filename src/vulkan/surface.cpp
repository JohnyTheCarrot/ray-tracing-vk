#include "surface.h"
#include "VkBootstrap.h"
#include "src/diagnostics.h"
#include "src/window.h"
#include <format>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	VkSurfaceDestroyer::VkSurfaceDestroyer(VkInstance instance)
	    : instance_{instance} {
	}

	void VkSurfaceDestroyer::operator()(VkSurfaceKHR surface) const {
		Logger::get_instance().log(LogLevel::Debug, "Destroying surface");
		vkDestroySurfaceKHR(instance_, surface, nullptr);
	}

	Surface::Surface(vkb::Instance &instance, VkSurfaceKHR surface)
	    : surface_{surface, VkSurfaceDestroyer{instance.instance}}
	    , instance_{&instance} {
	}

	vkb::PhysicalDevice Surface::select_physical_device() {
		vkb::PhysicalDeviceSelector phys_device_selector{*instance_};

		auto device_selector_return = phys_device_selector.set_surface(surface_.get())
		                                      .prefer_gpu_device_type(vkb::PreferredDeviceType::integrated)
		                                      .add_required_extension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME)
		                                      .add_required_extension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)
		                                      .add_required_extension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME)
		                                      .select();

		if (!device_selector_return) {
			std::string message{
			        std::format("Failed to select physical device: {}", device_selector_return.error().message())
			};
			throw std::runtime_error{std::move(message)};
		}

		return device_selector_return.value();
	}
}// namespace raytracing::vulkan
