#include "src/vulkan/vkb_raii.h"
#include "VkBootstrap.h"
#include "src/diagnostics.h"

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

	VkFenceDestroyer::VkFenceDestroyer(VkDevice device)
	    : device_{device} {
	}

	void VkFenceDestroyer::operator()(VkFence fence) {
		Logger::get_instance().log(LogLevel::Debug, "Destroying fence");
		vkDestroyFence(device_, fence, nullptr);
	}
};// namespace raytracing::vulkan
