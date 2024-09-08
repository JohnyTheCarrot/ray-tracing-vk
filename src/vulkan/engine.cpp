#include "engine.h"
#include "src/diagnostics.h"
#include "src/vulkan/command_pool.h"
#include "src/vulkan/logical_device.h"
#include "src/vulkan/phys_device.h"
#include <format>
#include <stdexcept>

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

	Engine::Engine(std::string_view app_name)
	    : instance_{[&] {
		    vkb::InstanceBuilder builder{};
		    auto const           instance_build_result = builder.set_app_name("Vulkan Ray Tracer")
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
	    , surface_{&window_.get_surface()}
	    , physical_device_{surface_->select_physical_device()}
	    , logical_device_{physical_device_.create_logical_device()}
	    , command_pool_{[&] {
		    auto const graphics_queue_idx{logical_device_.get().get_queue_index(vkb::QueueType::graphics)};
		    if (!graphics_queue_idx) {
			    std::string message{
			            std::format("Failed to get graphics queue index: {}", graphics_queue_idx.error().message())
			    };
			    throw std::runtime_error{std::move(message)};
		    }

		    return CommandPool{graphics_queue_idx.value(), logical_device_};
	    }()}
	    , swapchain_{logical_device_}
	    , allocator_{instance_.get(), physical_device_, logical_device_} {
	}

	VmaAllocator Engine::get_allocator() const {
		return allocator_.get();
	}

	LogicalDevice &Engine::get_logical_device() {
		return logical_device_;
	}

	LogicalDevice const &Engine::get_logical_device() const {
		return logical_device_;
	}

	CommandPool &Engine::get_command_pool() {
		return command_pool_;
	}

	CommandPool const &Engine::get_command_pool() const {
		return command_pool_;
	}

	void Engine::main_loop() {
		window_.main_loop();
	}
}// namespace raytracing::vulkan
