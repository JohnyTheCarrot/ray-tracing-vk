#ifndef SRC_VULKAN_PIPELINE_LAYOUT_H_
#define SRC_VULKAN_PIPELINE_LAYOUT_H_

#include "src/vulkan/vkb_raii.h"

namespace raytracing::vulkan {
	class PipelineLayout final {
		UniqueVkPipelineLayout pipeline_layout_;

	public:
		explicit PipelineLayout(VkDevice device);

		[[nodiscard]]
		VkPipelineLayout get() const;
	};
}// namespace raytracing::vulkan

#endif//  SRC_VULKAN_PIPELINE_LAYOUT_H_
