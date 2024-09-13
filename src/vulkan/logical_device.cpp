#include "logical_device.h"
#include "allocator.h"
#include "buffer.h"
#include "external/stb_image.h"
#include "phys_device.h"
#include "src/vulkan/image.h"
#include "src/vulkan/vkb_raii.h"
#include "vk_exception.h"
#include <format>
#include <vulkan/vulkan_core.h>

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

	Image LogicalDevice::create_image(
	        Allocator const &allocator, std::uint32_t width, std::uint32_t height, VkFormat format,
	        VkImageUsageFlags usage_flags
	) const {
		VkImageCreateInfo create_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
		create_info.format        = format;
		create_info.usage         = usage_flags;
		create_info.tiling        = VK_IMAGE_TILING_OPTIMAL;
		create_info.imageType     = VK_IMAGE_TYPE_2D;
		create_info.samples       = VK_SAMPLE_COUNT_1_BIT;
		create_info.mipLevels     = 1;
		create_info.arrayLayers   = 1;
		create_info.extent.width  = width;
		create_info.extent.height = height;
		create_info.extent.depth  = 1;
		create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo alloc_info{};
		alloc_info.usage         = VMA_MEMORY_USAGE_AUTO;
		alloc_info.flags         = 0;
		alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		VkImage       image{};
		VmaAllocation allocation{};

		if (VkResult const result{
		            vmaCreateImage(allocator.get(), &create_info, &alloc_info, &image, &allocation, nullptr)
		    };
		    result != VK_SUCCESS) {
			throw VkException{"Could not create image", result};
		}

		UniqueVkImage unique_image{image, VkImageDestroyer{allocator.get(), allocation}};

		return Image{std::move(unique_image), width, height, device_.get(), format};
	}

	class StbiImageDestroyer final {
	public:
		void operator()(stbi_uc *pixels) const {
			stbi_image_free(pixels);
		}
	};

	Image LogicalDevice::create_image(
	        CommandPool const &command_pool, std::filesystem::path const &path, Allocator const &allocator,
	        VkFormat format, VkImageUsageFlags usage_flags
	) const {
		int                                          width, height, texChannels;
		std::unique_ptr<stbi_uc, StbiImageDestroyer> pixels{
		        stbi_load(path.c_str(), &width, &height, &texChannels, STBI_rgb_alpha)
		};
		std::size_t const size{static_cast<std::size_t>(width * height * 4)};

		Image image{create_image(
		        allocator, static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height), format,
		        VK_IMAGE_USAGE_TRANSFER_DST_BIT | usage_flags
		)};

		Buffer staging_buffer{
		        device_.get().device,
		        allocator.get(),
		        std::span{pixels.get(), size},
		        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		auto const mem{staging_buffer.map_memory()};
		memcpy(mem.get_mapped_ptr(), pixels.get(), size);
		image.transition_layout(
		        command_pool, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT
		);

		staging_buffer.copy_to(command_pool, image);
		image.transition_layout(
		        command_pool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		        VK_IMAGE_ASPECT_COLOR_BIT
		);

		return image;
	}
}// namespace raytracing::vulkan
