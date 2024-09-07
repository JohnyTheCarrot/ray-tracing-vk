#ifndef SRC_VULKAN_LOGICAL_DEVICE_H_
#define SRC_VULKAN_LOGICAL_DEVICE_H_

#include "vkb_raii.h"

namespace raytracing::vulkan {
	class LogicalDevice final {
		UniqueVkbDevice device_;

	public:
		explicit LogicalDevice(UniqueVkbDevice &&device);
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_LOGICAL_DEVICE_H_
