#include "instance.h"

#include "src/diagnostics.h"
#include "vk_exception.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

namespace raytracing::vulkan {
	void VkInstanceDestroyer::operator()(VkInstance instance) const {
		Logger::GetInstance().Log(LogLevel::Debug, "Destroying Vulkan instance");
		vkDestroyInstance(instance, nullptr);
	}

	Instance::Instance(std::string_view applicationName, std::string_view engineName) {
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
		const char  **glfwExtensions{glfwGetRequiredInstanceExtensions(&glfwExtensionCount)};
		instanceCreateInfo.enabledExtensionCount   = glfwExtensionCount;
		instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;
		instanceCreateInfo.enabledLayerCount       = 0;

		VkInstance instance{nullptr};
		Logger::GetInstance().Log(LogLevel::Debug, "Creating Vulkan instance");
		if (const VkResult result{vkCreateInstance(&instanceCreateInfo, nullptr, &instance)}; result != VK_SUCCESS) {
			throw VkException{"Failed to create Vulkan instance", result};
		}

		m_Instance.reset(instance);
	}
}// namespace raytracing::vulkan
