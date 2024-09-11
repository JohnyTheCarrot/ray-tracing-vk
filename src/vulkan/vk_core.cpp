#include "vk_core.h"
#include "src/diagnostics.h"
#include "src/vulkan/device_manager.h"
#include <format>

namespace raytracing::vulkan {
	VkBool32 debug_callback(
	        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type,
	        VkDebugUtilsMessengerCallbackDataEXT const *callback_data, void *user_data
	) {
		auto const severity{vkb::to_string_message_severity(message_severity)};
		auto const type{vkb::to_string_message_type(message_type)};
		auto const is_error{message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT};

		std::string complete_message{std::format("[{}: {}] {}", severity, type, callback_data->pMessage)};

		auto &logger_instance{Logger::get_instance()};
		if (is_error) {
			logger_instance.error(complete_message);
		} else {
			logger_instance.log(complete_message);
		}

		return VK_FALSE;
	}

	VulkanCore::VulkanCore(std::string_view app_name)
	    : instance_{[&] {
		    vkb::InstanceBuilder builder{};
		    auto const           instance_build_result = builder.set_app_name(app_name.data())
		                                               .set_engine_name("Roingus Engine")
		                                               .request_validation_layers()
		                                               .require_api_version(1, 2, 0)
		                                               .set_debug_callback(debug_callback)
		                                               .build();

		    if (!instance_build_result) {
			    std::string message{std::format("Failed to build instance: {}", instance_build_result.error().message())
			    };
			    throw std::runtime_error{std::move(message)};
		    }

		    return vulkan::UniqueVkbInstance{instance_build_result.value()};
	    }()}
	    , window_{instance_.get(), 800, 600, app_name.data()}
	    , surface_{&window_.get_surface()} {
	}

	DeviceManager VulkanCore::create_device_manager() const {
		return DeviceManager{instance_.get(), surface_->select_physical_device()};
	}

	bool VulkanCore::get_close_requested() const {
		return window_.get_should_close();
	}

	void VulkanCore::update() {
		window_.poll_events();
	}
}// namespace raytracing::vulkan
