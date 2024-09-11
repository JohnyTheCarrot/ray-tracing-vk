#include "rasterizer.h"
#include "logical_device.h"
#include <vector>
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	Rasterizer::Rasterizer(LogicalDevice const &logical_device, Swapchain const &swapchain)
	    : vert_shader_module_{logical_device.get(), "shaders/shader.vert.spv"}
	    , frag_shader_module_{logical_device.get(), "shaders/shader.frag.spv"}
	    , pipeline_{
	              logical_device, swapchain,
	              std::vector{
	                      vert_shader_module_.create_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT),
	                      frag_shader_module_.create_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT)
	              }
	      } {
	}

	void Rasterizer::render() const {
		pipeline_.render();
	}
}// namespace raytracing::vulkan
