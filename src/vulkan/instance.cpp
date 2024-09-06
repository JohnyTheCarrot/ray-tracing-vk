#include "instance.h"

#include "src/diagnostics.h"
#include "vk_exception.h"

#include <GLFW/glfw3.h>
#include <cstdint>
#include <vulkan/vulkan.hpp>

namespace raytracing::vulkan {
	std::vector const validationLayers = {"VK_LAYER_KHRONOS_validation"};

	void VkInstanceDestroyer::operator()(VkInstance instance) const {
		Logger::get_instance().Log(LogLevel::Debug, "Destroying Vulkan instance");
		vkDestroyInstance(instance, nullptr);
	}

	bool Instance::CheckValidationLayerSupport() {
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

	Instance::Instance(std::string_view applicationName, std::string_view engineName, bool enableValidationLayers) {
		if (enableValidationLayers && !CheckValidationLayerSupport()) {
			throw std::runtime_error{"Validation layers requested, but not available."};
		}

		VkApplicationInfo applicationInfo{};
		applicationInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pApplicationName   = applicationName.data();
		applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		applicationInfo.pEngineName        = engineName.data();
		applicationInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
		applicationInfo.apiVersion         = VK_API_VERSION_1_0;

		VkInstanceCreateInfo instanceCreateInfo{};
		instanceCreateInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pApplicationInfo = &applicationInfo;

		std::uint32_t glfwExtensionCount{0};
		char const  **glfwExtensions{glfwGetRequiredInstanceExtensions(&glfwExtensionCount)};
		instanceCreateInfo.enabledExtensionCount   = glfwExtensionCount;
		instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;

		if (enableValidationLayers) {
			instanceCreateInfo.enabledLayerCount   = static_cast<std::uint32_t>(validationLayers.size());
			instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		} else {
			instanceCreateInfo.enabledLayerCount = 0;
		}

		VkInstance instance{nullptr};
		Logger::get_instance().Log(LogLevel::Debug, "Creating Vulkan instance");
		if (VkResult const result{vkCreateInstance(&instanceCreateInfo, nullptr, &instance)}; result != VK_SUCCESS) {
			throw VkException{"Failed to create Vulkan instance", result};
		}

		m_Instance.reset(instance);
	}
}// namespace raytracing::vulkan
