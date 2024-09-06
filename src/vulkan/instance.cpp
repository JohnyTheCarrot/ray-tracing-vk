#include "instance.h"

#include "src/diagnostics.h"
#include "vk_exception.h"

#include <GLFW/glfw3.h>
#include <cstdint>
#include <vulkan/vulkan.hpp>

namespace raytracing::vulkan {
	std::vector const validationLayers = {"VK_LAYER_KHRONOS_validation"};

	void VkInstanceDestroyer::operator()(VkInstance instance) const {
		Logger::get_instance().log(LogLevel::Debug, "Destroying Vulkan instance");
		vkDestroyInstance(instance, nullptr);
	}

	bool Instance::check_validation_layer_support() {
		std::uint32_t layerCount{0};
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		return std::ranges::all_of(validationLayers, [&availableLayers](std::string const &layer) {
			return std::ranges::any_of(availableLayers, [&layer](const VkLayerProperties &availableLayer) {
				return layer == availableLayer.layerName;
			});
		});
	}

	Instance::Instance(std::string_view application_name, std::string_view engine_name, bool enable_validation_layers) {
		if (enable_validation_layers && !check_validation_layer_support()) {
			throw std::runtime_error{"Validation layers requested, but not available."};
		}

		VkApplicationInfo applicationInfo{};
		applicationInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pApplicationName   = application_name.data();
		applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		applicationInfo.pEngineName        = engine_name.data();
		applicationInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
		applicationInfo.apiVersion         = VK_API_VERSION_1_0;

		VkInstanceCreateInfo instance_create_info{};
		instance_create_info.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pApplicationInfo = &applicationInfo;

		std::uint32_t glfw_extension_count{0};
		char const  **glfw_extensions{glfwGetRequiredInstanceExtensions(&glfw_extension_count)};
		instance_create_info.enabledExtensionCount   = glfw_extension_count;
		instance_create_info.ppEnabledExtensionNames = glfw_extensions;

		if (enable_validation_layers) {
			instance_create_info.enabledLayerCount   = static_cast<std::uint32_t>(validationLayers.size());
			instance_create_info.ppEnabledLayerNames = validationLayers.data();
		} else {
			instance_create_info.enabledLayerCount = 0;
		}

		VkInstance instance{nullptr};
		Logger::get_instance().log(LogLevel::Debug, "Creating Vulkan instance");
		if (VkResult const result{vkCreateInstance(&instance_create_info, nullptr, &instance)}; result != VK_SUCCESS) {
			throw VkException{"Failed to create Vulkan instance", result};
		}

		instance_.reset(instance);
	}

	VkInstance Instance::get() const noexcept {
		return instance_.get();
	}
}// namespace raytracing::vulkan
