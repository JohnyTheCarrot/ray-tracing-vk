#include "descriptor_pool.h"
#include "src/diagnostics.h"
#include "src/vulkan/vk_exception.h"
#include <algorithm>
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	VkDescriptorPoolDestroyer::VkDescriptorPoolDestroyer(VkDevice device)
	    : device_{device} {
	}

	void VkDescriptorPoolDestroyer::operator()(VkDescriptorPool descriptor_pool) const {
		Logger::get_instance().log(LogLevel::Debug, "Destroying descriptor pool");
		vkDestroyDescriptorPool(device_, descriptor_pool, nullptr);
	}

	DescriptorPool::DescriptorPool(
	        VkDevice device, std::vector<VkDescriptorSetLayoutBinding> const &bindings, std::uint32_t max_sets
	)
	    : device_{device}
	    , descriptor_pool_{[&] {
		    std::vector<VkDescriptorPoolSize> pool_sizes{};
		    for (const auto &binding: bindings) {
			    if (binding.descriptorCount == 0)
				    continue;

			    auto const existing_pool_size{std::find_if(
			            pool_sizes.begin(), pool_sizes.end(),
			            [&](VkDescriptorPoolSize pool_size) { return pool_size.type == binding.descriptorType; }
			    )};

			    if (existing_pool_size != pool_sizes.end()) {
				    existing_pool_size->descriptorCount += binding.descriptorCount;
				    continue;
			    }

			    VkDescriptorPoolSize pool_size{};
			    pool_size.type            = binding.descriptorType;
			    pool_size.descriptorCount = binding.descriptorCount;
			    pool_sizes.emplace_back(pool_size);
		    }

		    VkDescriptorPoolCreateInfo desc_pool_info{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
		    desc_pool_info.pNext         = VK_NULL_HANDLE;
		    desc_pool_info.maxSets       = max_sets;
		    desc_pool_info.poolSizeCount = pool_sizes.size();
		    desc_pool_info.pPoolSizes    = pool_sizes.data();
		    desc_pool_info.flags         = 0;

		    VkDescriptorPool descriptor_pool{};
		    if (VkResult const result{vkCreateDescriptorPool(device, &desc_pool_info, nullptr, &descriptor_pool)};
		        result != VK_SUCCESS) {
			    throw VkException{"Could not create descriptor pool", result};
		    }

		    return UniqueVkDescriptorPool{descriptor_pool, VkDescriptorPoolDestroyer{device}};
	    }()} {
	}

	VkDescriptorSet DescriptorPool::create_descriptor_set(VkDescriptorSetLayout desc_set_layout) {
		VkDescriptorSetAllocateInfo allocate_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
		allocate_info.descriptorPool     = descriptor_pool_.get();
		allocate_info.descriptorSetCount = 1;
		allocate_info.pSetLayouts        = &desc_set_layout;

		VkDescriptorSet desc_set{};
		if (VkResult const result{vkAllocateDescriptorSets(device_, &allocate_info, &desc_set)}; result != VK_SUCCESS) {
			throw VkException{"Could not allocate descriptor sets", result};
		}

		return desc_set;
	}
}// namespace raytracing::vulkan
