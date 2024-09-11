#ifndef SRC_VULKAN_VK_CORE_H_
#define SRC_VULKAN_VK_CORE_H_

#include "src/vulkan/device_manager.h"
#include "src/vulkan/surface.h"
#include "src/vulkan/vkb_raii.h"
#include "src/window.h"
#include <string_view>

namespace raytracing::vulkan {
	class VulkanCore final {
		UniqueVkbInstance instance_;
		Window            window_;
		Surface          *surface_;

	public:
		explicit VulkanCore(std::string_view app_name);

		[[nodiscard]]
		DeviceManager create_device_manager() const;

		[[nodiscard]]
		bool get_close_requested() const;

		void update();
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_VK_CORE_H_
