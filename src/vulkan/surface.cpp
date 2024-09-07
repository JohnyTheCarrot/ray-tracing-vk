#include "surface.h"
#include "VkBootstrap.h"
#include "src/diagnostics.h"
#include "src/window.h"
#include <format>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	std::vector<char const *> const required_extensions{
	        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,   VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
	        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, VK_KHR_SPIRV_1_4_EXTENSION_NAME,
	        VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
	        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
	};

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

	PhysicalDevice Surface::select_physical_device() {
		vkb::PhysicalDeviceSelector phys_device_selector{*instance_};

		auto device_selector_return = phys_device_selector.set_surface(surface_.get())
		                                      .prefer_gpu_device_type(vkb::PreferredDeviceType::integrated)
		                                      .add_required_extensions(required_extensions)
		                                      .select();

		if (!device_selector_return) {
			std::string message{
			        std::format("Failed to select physical device: {}", device_selector_return.error().message())
			};
			throw std::runtime_error{std::move(message)};
		}

		return PhysicalDevice{std::move(device_selector_return.value())};
	}
}// namespace raytracing::vulkan
