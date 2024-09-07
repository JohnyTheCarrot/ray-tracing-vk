#include "logical_device.h"

namespace raytracing::vulkan {
	LogicalDevice::LogicalDevice(UniqueVkbDevice &&device)
	    : device_{std::move(device)} {
	}
}// namespace raytracing::vulkan
