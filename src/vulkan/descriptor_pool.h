#include <cstdint>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	class VkDescriptorPoolDestroyer final {
		VkDevice device_;

	public:
		explicit VkDescriptorPoolDestroyer(VkDevice device);

		void operator()(VkDescriptorPool descriptor_pool) const;
	};

	using UniqueVkDescriptorPool = std::unique_ptr<VkDescriptorPool_T, VkDescriptorPoolDestroyer>;

	class DescriptorPool final {
		UniqueVkDescriptorPool descriptor_pool_;
		VkDevice               device_;

	public:
		DescriptorPool(
		        VkDevice device, std::vector<VkDescriptorSetLayoutBinding> const &bindings, std::uint32_t max_sets
		);

		[[nodiscard]]
		VkDescriptorSet create_descriptor_set(VkDescriptorSetLayout desc_set_layout);
	};
}// namespace raytracing::vulkan
