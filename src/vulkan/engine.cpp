#include "engine.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VkBootstrap.h"
#include "src/diagnostics.h"
#include "src/vulkan/command_pool.h"
#include "src/vulkan/graphics_pipeline.h"
#include "src/vulkan/logical_device.h"
#include "src/vulkan/phys_device.h"
#include <filesystem>
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

	std::vector<char> read_file(std::filesystem::path const &path) {
		std::ifstream file{path, std::ios::binary | std::ios::ate};

		if (!file) {
			throw std::runtime_error{std::string{"couldn't open file " + path.string()}};
		}

		size_t            filesize = (size_t) file.tellg();
		std::vector<char> buffer(filesize);
		file.seekg(0);
		file.read(buffer.data(), filesize);

		file.close();
		return buffer;
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
		    auto const graphics_queue_idx{logical_device_.get_queue_index(vkb::QueueType::graphics)};

		    return CommandPool{graphics_queue_idx, logical_device_};
	    }()}
	    , swapchain_{logical_device_}
	    , allocator_{instance_.get(), physical_device_, logical_device_}
	    , vert_shader_module_{[&] {
		    auto vert_shader_bytecode{read_file("shaders/shader.vert.spv")};
		    return ShaderModule{logical_device_.get(), vert_shader_bytecode};
	    }()}
	    , frag_shader_module_{[&] {
		    auto frag_shader_bytecode{read_file("shaders/shader.frag.spv")};
		    return ShaderModule{logical_device_.get(), frag_shader_bytecode};
	    }()}
	    , graphics_pipeline_{
	              logical_device_, swapchain_,
	              std::vector{
	                      vert_shader_module_.create_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT),
	                      frag_shader_module_.create_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT)
	              }
	      } {
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
		while (!glfwWindowShouldClose(window_.get())) {
			glfwPollEvents();
			graphics_pipeline_.render();
		}
	}
}// namespace raytracing::vulkan
