#include "src/vulkan/vkb_raii.h"
#include "VkBootstrap.h"
#include "src/diagnostics.h"

namespace raytracing {
	void VkbInstanceDestroyer::operator()(vkb::Instance const &instance) {
		Logger::get_instance().log(LogLevel::Debug, "Destroying instance");
		vkb::destroy_instance(instance);
	}

	void VkbDeviceDestroyer::operator()(vkb::Device const &device) {
		Logger::get_instance().log(LogLevel::Debug, "Destroying logical device");
		vkb::destroy_device(device);
	}
};// namespace raytracing
