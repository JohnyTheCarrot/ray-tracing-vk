#include "src/vulkan/vkb_raii.h"
#include "VkBootstrap.h"
#include "src/diagnostics.h"
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	void VkbInstanceDestroyer::operator()(vkb::Instance const &instance) {
		Logger::get_instance().log(LogLevel::Debug, "Destroying instance");
		vkb::destroy_instance(instance);
	}

	void VkbDeviceDestroyer::operator()(vkb::Device const &device) {
		Logger::get_instance().log(LogLevel::Debug, "Destroying logical device");
		vkb::destroy_device(device);
	}

	void VkbSwapchainDestroyer::operator()(vkb::Swapchain const &swapchain) {
		Logger::get_instance().log(LogLevel::Debug, "Destroying swapchain");
		vkb::destroy_swapchain(swapchain);
	}

	void VmaAllocatorDestroyer::operator()(VmaAllocator const &allocator) {
		Logger::get_instance().log(LogLevel::Debug, "Destroying VmaAllocator");
		vmaDestroyAllocator(allocator);
	}

	VkShaderModuleDestroyer::VkShaderModuleDestroyer(VkDevice device)
	    : device_{device} {
	}

	void VkShaderModuleDestroyer::operator()(VkShaderModule shader_module) const {
		Logger::get_instance().log(LogLevel::Debug, "Destroying shader module");
		vkDestroyShaderModule(device_, shader_module, nullptr);
	}

	VkPipelineLayoutDestroyer::VkPipelineLayoutDestroyer(VkDevice device)
	    : device_{device} {
	}

	void VkPipelineLayoutDestroyer::operator()(VkPipelineLayout pipeline_layout) const {
		Logger::get_instance().log(LogLevel::Debug, "Destroying pipeline layout");
		vkDestroyPipelineLayout(device_, pipeline_layout, nullptr);
	}
};// namespace raytracing::vulkan
