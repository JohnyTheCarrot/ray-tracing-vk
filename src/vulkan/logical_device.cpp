#include "logical_device.h"
#include "phys_device.h"
#include "vk_exception.h"
#include <format>

namespace raytracing::vulkan {
	LogicalDevice::LogicalDevice(UniqueVkbDevice &&device, PhysicalDevice const &phys_device)
	    : device_{std::move(device)}
	    , phys_device_{&phys_device} {
	}

	vkb::Device &LogicalDevice::get() noexcept {
		return device_.get();
	}

	vkb::Device const &LogicalDevice::get() const noexcept {
		return device_.get();
	}

	PhysicalDevice const &LogicalDevice::get_phys() const noexcept {
		return *phys_device_;
	}

	VkQueue LogicalDevice::get_queue(std::uint32_t index) const noexcept {
		VkQueue queue;
		vkGetDeviceQueue(device_.get().device, index, 0, &queue);

		return queue;
	}

	VkQueue LogicalDevice::get_queue(vkb::QueueType queue_type) const {
		auto const queue_idx{get().get_queue(queue_type)};
		if (!queue_idx) {
			std::string message{std::format("Failed to get queue: {}", queue_idx.error().message())};
			throw std::runtime_error{std::move(message)};
		}

		return queue_idx.value();
	}

	std::uint32_t LogicalDevice::get_queue_index(vkb::QueueType queue_type) const {
		auto const queue_idx{get().get_queue_index(queue_type)};
		if (!queue_idx) {
			std::string message{std::format("Failed to get queue index: {}", queue_idx.error().message())};
			throw std::runtime_error{std::move(message)};
		}

		return queue_idx.value();
	}

	void LogicalDevice::wait_idle() const {
		vkDeviceWaitIdle(device_.get());
	}

	UniqueVkSemaphore LogicalDevice::create_semaphore() const {
		VkSemaphoreCreateInfo semaphore_info{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

		VkSemaphore semaphore{};
		if (VkResult const result{vkCreateSemaphore(device_.get(), &semaphore_info, nullptr, &semaphore)};
		    result != VK_SUCCESS) {
			throw VkException{"Could not create semaphore", result};
		}

		return UniqueVkSemaphore{semaphore, VkSemaphoreDestroyer{device_.get()}};
	}

	UniqueVkFence LogicalDevice::create_fence(VkFenceCreateFlags flags) const {
		VkFenceCreateInfo fence_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
		fence_info.flags = flags;

		VkFence fence{};
		if (VkResult const result{vkCreateFence(device_.get(), &fence_info, nullptr, &fence)}; result != VK_SUCCESS) {
			throw VkException{"Could not create fence", result};
		}

		return vulkan::UniqueVkFence{fence, vulkan::VkFenceDestroyer{device_.get()}};
	}

	UniqueVkDescriptorSetLayout LogicalDevice::create_descriptor_set_layout(
	        std::vector<VkDescriptorBindingFlags> const  &binding_flags,
	        std::span<VkDescriptorSetLayoutBinding const> bindings
	) const {
		auto final_binding_flags{binding_flags};
		if (binding_flags.empty() || binding_flags.size() < bindings.size()) {
			final_binding_flags.resize(bindings.size(), 0);
		}

		VkDescriptorSetLayoutBindingFlagsCreateInfo bindings_info{
		        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO
		};
		bindings_info.bindingCount  = static_cast<std::uint32_t>(bindings.size());
		bindings_info.pBindingFlags = final_binding_flags.data();

		VkDescriptorSetLayoutCreateInfo create_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
		create_info.bindingCount = bindings_info.bindingCount;
		create_info.pBindings    = bindings.data();
		create_info.flags        = 0;
		create_info.pNext        = final_binding_flags.empty() ? nullptr : &bindings_info;

		VkDescriptorSetLayout desc_set_layout{};
		if (VkResult const result{
		            vkCreateDescriptorSetLayout(device_.get().device, &create_info, nullptr, &desc_set_layout)
		    };
		    result != VK_SUCCESS) {
			throw VkException{"Failed to create descriptor set layout", result};
		}

		return UniqueVkDescriptorSetLayout{desc_set_layout, VkDescriptorSetLayoutDestroyer{device_.get().device}};
	}
}// namespace raytracing::vulkan
