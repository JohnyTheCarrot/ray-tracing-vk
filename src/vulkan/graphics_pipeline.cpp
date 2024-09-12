#include "graphics_pipeline.h"
#include "src/diagnostics.h"
#include "src/vulkan/host_device.h"
#include "src/vulkan/logical_device.h"
#include "src/vulkan/swapchain.h"
#include "src/vulkan/vk_exception.h"
#include <array>
#include <vulkan/vulkan_core.h>

namespace raytracing::vulkan {
	VkPipelineDestroyer::VkPipelineDestroyer(VkDevice device)
	    : device_{device} {
	}

	void VkPipelineDestroyer::operator()(VkPipeline pipeline) const {
		Logger::get_instance().log(LogLevel::Debug, "Destroying pipeline");
		vkDestroyPipeline(device_, pipeline, nullptr);
	}

	GraphicsPipeline::GraphicsPipeline(
	        LogicalDevice const &device, Allocator const &allocator, Swapchain const &swapchain
	)
	    : render_pass_{device, allocator, swapchain}
	    , vert_shader_module_{device.get(), "shaders/shader.vert.spv"}
	    , frag_shader_module_{device.get(), "shaders/shader.frag.spv"}
	    , pipeline_layout_{[&] {
		    VkPipelineLayoutCreateInfo pipeline_layout_info{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

		    VkPushConstantRange push_constant_range{};
		    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		    push_constant_range.offset     = 0;
		    push_constant_range.size       = sizeof(glm::mat4);

		    VkDescriptorSetLayout layout{render_pass_.get_desc_set_manager().get_layout()};
		    pipeline_layout_info.setLayoutCount         = 1;
		    pipeline_layout_info.pSetLayouts            = &layout;
		    pipeline_layout_info.pushConstantRangeCount = 1;
		    pipeline_layout_info.pPushConstantRanges    = &push_constant_range;

		    VkPipelineLayout pipeline_layout{};
		    if (VkResult const result{
		                vkCreatePipelineLayout(device.get(), &pipeline_layout_info, nullptr, &pipeline_layout)
		        };
		        result != VK_SUCCESS) {
			    throw VkException{"Couldn't create pipeline layout", result};
		    }

		    return UniqueVkPipelineLayout{pipeline_layout, VkPipelineLayoutDestroyer{device.get()}};
	    }()}
	    , pipeline_{[&] {
		    std::array<VkDynamicState, 2> dynamic_states{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

		    VkPipelineDynamicStateCreateInfo dynamic_state{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
		    dynamic_state.dynamicStateCount = dynamic_states.size();
		    dynamic_state.pDynamicStates    = dynamic_states.data();

		    VkVertexInputAttributeDescription vert_attr_desc{};
		    vert_attr_desc.binding  = 0;
		    vert_attr_desc.location = 0;
		    vert_attr_desc.format   = VK_FORMAT_R32G32B32_SFLOAT;
		    vert_attr_desc.offset   = offsetof(Vertex, pos);

		    std::vector<VkVertexInputAttributeDescription> attr_descs(5);
		    attr_descs[0] = vert_attr_desc;
		    for (int i{}; i < sizeof(glm::mat4) / sizeof(glm::vec4); ++i) {
			    attr_descs[1 + i].binding  = 1;
			    attr_descs[1 + i].location = 1 + i;
			    attr_descs[1 + i].format   = VK_FORMAT_R32G32B32A32_SFLOAT;
			    attr_descs[1 + i].offset   = sizeof(glm::vec4) * i;
		    }

		    VkVertexInputBindingDescription vert_binding_description{};
		    vert_binding_description.binding   = 0;
		    vert_binding_description.stride    = sizeof(Vertex);
		    vert_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		    VkVertexInputBindingDescription instance_binding_description{};
		    instance_binding_description.binding   = 1;
		    instance_binding_description.stride    = sizeof(glm::mat4);
		    instance_binding_description.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

		    std::array bindings{vert_binding_description, instance_binding_description};

		    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
		    vertex_input_info.sType                         = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		    vertex_input_info.vertexBindingDescriptionCount = bindings.size();
		    vertex_input_info.pVertexBindingDescriptions    = bindings.data();
		    vertex_input_info.vertexAttributeDescriptionCount = attr_descs.size();
		    vertex_input_info.pVertexAttributeDescriptions    = attr_descs.data();

		    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
		    input_assembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		    input_assembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		    input_assembly.primitiveRestartEnable = VK_FALSE;

		    VkExtent2D swapchain_extent{swapchain.get().extent};

		    VkViewport viewport{};
		    viewport.x        = 0.0f;
		    viewport.y        = 0.0f;
		    viewport.width    = static_cast<float>(swapchain_extent.width);
		    viewport.height   = static_cast<float>(swapchain_extent.height);
		    viewport.minDepth = 0.0f;
		    viewport.maxDepth = 1.0f;

		    VkRect2D scissor{};
		    scissor.offset = {0, 0};
		    scissor.extent = swapchain_extent;

		    VkPipelineViewportStateCreateInfo viewport_state{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
		    viewport_state.viewportCount = 1;
		    viewport_state.scissorCount  = 1;

		    VkPipelineRasterizationStateCreateInfo rasterizer{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
		    };
		    rasterizer.depthClampEnable        = VK_FALSE;
		    rasterizer.rasterizerDiscardEnable = VK_FALSE;
		    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
		    rasterizer.lineWidth               = 1.0f;
		    rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
		    rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		    rasterizer.depthBiasEnable         = VK_FALSE;

		    VkPipelineMultisampleStateCreateInfo multisampling{};
		    multisampling.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		    multisampling.sampleShadingEnable  = VK_FALSE;
		    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		    VkPipelineColorBlendAttachmentState color_blend_attachment{};
		    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		                                            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		    color_blend_attachment.blendEnable         = VK_TRUE;
		    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		    color_blend_attachment.colorBlendOp        = VK_BLEND_OP_ADD;
		    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		    color_blend_attachment.alphaBlendOp        = VK_BLEND_OP_ADD;

		    VkPipelineColorBlendStateCreateInfo color_blending{};
		    color_blending.sType         = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		    color_blending.logicOpEnable = VK_FALSE;

		    color_blending.attachmentCount = 1;
		    color_blending.pAttachments    = &color_blend_attachment;

		    std::vector<VkPipelineShaderStageCreateInfo> const shader_stages{
		            vert_shader_module_.create_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT),
		            frag_shader_module_.create_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT)
		    };
		    VkGraphicsPipelineCreateInfo pipeline_info{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
		    pipeline_info.stageCount          = shader_stages.size();
		    pipeline_info.pStages             = shader_stages.data();
		    pipeline_info.pVertexInputState   = &vertex_input_info;
		    pipeline_info.pInputAssemblyState = &input_assembly;
		    pipeline_info.pViewportState      = &viewport_state;
		    pipeline_info.pRasterizationState = &rasterizer;
		    pipeline_info.pMultisampleState   = &multisampling;
		    pipeline_info.pDepthStencilState  = nullptr;
		    pipeline_info.pColorBlendState    = &color_blending;
		    pipeline_info.pDynamicState       = &dynamic_state;

		    pipeline_info.layout     = pipeline_layout_.get();
		    pipeline_info.renderPass = render_pass_.get_render_pass().get();
		    pipeline_info.subpass    = 0;

		    VkPipeline graphics_pipeline{};
		    if (VkResult const result{vkCreateGraphicsPipelines(
		                device.get(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline
		        )};
		        result != VK_SUCCESS) {
			    throw VkException{"Could not create graphics pipeline", result};
		    }

		    return UniqueVkPipeline{graphics_pipeline, VkPipelineDestroyer{device.get()}};
	    }()} {
	}

	void GraphicsPipeline::render(Scene const &scene) const {
		render_pass_.render(pipeline_.get(), pipeline_layout_.get(), scene);
	}
}// namespace raytracing::vulkan
