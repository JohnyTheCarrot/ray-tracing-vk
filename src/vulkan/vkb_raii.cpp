#include "src/vulkan/vkb_raii.h"
#include "src/diagnostics.h"

namespace raytracing {
	void VkbInstanceDestroyer::operator()(vkb::Instance const &instance) {
		Logger::get_instance().log(LogLevel::Debug, "Destroying instance");
		vkb::destroy_instance(instance);
	}
};// namespace raytracing
