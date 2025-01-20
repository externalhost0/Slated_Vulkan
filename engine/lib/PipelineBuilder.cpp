//
// Created by Hayden Rivas on 1/16/25.
//
#include "Slate/Debug.h"
#include "Slate/VK/vkinfo.h"

#include "Slate/VK/PipelineBuilder.h"

namespace Slate {
	void PipelineBuilder::Clear() {
		// clear all of the structs we need back to 0 with their correct stype
		_inputAssembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };

		_rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };

		_colorBlendAttachment = {};

		_multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };

		_depthStencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };

		_renderInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };

		_shaderStages.clear();
	}
	// build() needs to be the last function called on the builder!
	VkPipeline PipelineBuilder::build(VkDevice device, VkPipelineLayout layout) {
		VkPipelineViewportStateCreateInfo viewportState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .pNext = nullptr };
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineColorBlendStateCreateInfo colorBlending = { .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, .pNext = nullptr };
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &_colorBlendAttachment;

		// completely clear VertexInputStateCreateInfo, as we have no need for it
		VkPipelineVertexInputStateCreateInfo _vertexInputInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, .pNext = nullptr };


		VkGraphicsPipelineCreateInfo pipelineInfo = { .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		pipelineInfo.pNext = &_renderInfo;

		// the shaders we added to the pipeline
		pipelineInfo.stageCount = static_cast<uint32_t>(_shaderStages.size());
		pipelineInfo.pStages = _shaderStages.data();

		pipelineInfo.pVertexInputState = &_vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &_inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &_rasterizer;
		pipelineInfo.pMultisampleState = &_multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDepthStencilState = &_depthStencil;
		pipelineInfo.layout = layout;

		VkDynamicState state[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicInfo = vkinfo::CreatePipelineDynamicStateInfo(state, 2);
		pipelineInfo.pDynamicState = &dynamicInfo;

		VkPipeline newPipeline;
		VK_CHECK(vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineInfo,nullptr, &newPipeline));
		return newPipeline;
	}


	PipelineBuilder& PipelineBuilder::set_shaders(VkShaderModule vertexShader, VkShaderModule fragmentShader) {
		_shaderStages.clear();

		_shaderStages.push_back(vkinfo::CreatePipelineShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT, vertexShader));
		_shaderStages.push_back(vkinfo::CreatePipelineShaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader));

		return *this;
	}
	PipelineBuilder& PipelineBuilder::set_input_topology(VkPrimitiveTopology topology) {
		_inputAssembly.topology = topology;
		_inputAssembly.primitiveRestartEnable = VK_FALSE;

		return *this;
	}
	PipelineBuilder& PipelineBuilder::set_polygon_mode(VkPolygonMode mode) {
		_rasterizer.polygonMode = mode;
		_rasterizer.lineWidth = 1.f;

		return *this;
	}
	PipelineBuilder& PipelineBuilder::set_cull_mode(VkCullModeFlags cullMode, VkFrontFace frontFace) {
		_rasterizer.cullMode = cullMode;
		_rasterizer.frontFace = frontFace;

		return *this;
	}
	PipelineBuilder& PipelineBuilder::set_multisampling_none() {
		_multisampling.sampleShadingEnable = VK_FALSE;
		_multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // msaa setting
		_multisampling.minSampleShading = 1.0f;
		_multisampling.pSampleMask = nullptr;
		_multisampling.alphaToCoverageEnable = VK_FALSE;
		_multisampling.alphaToOneEnable = VK_FALSE;

		return *this;
	}
	PipelineBuilder& PipelineBuilder::disable_blending() {
		// default write mask
		// we want to write to rgba
		_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		_colorBlendAttachment.blendEnable = VK_FALSE;

		return *this;
	}
	PipelineBuilder& PipelineBuilder::set_color_attachment_format(VkFormat format) {
		_colorAttachmentFormat = format;
		_renderInfo.colorAttachmentCount = 1;
		_renderInfo.pColorAttachmentFormats = &_colorAttachmentFormat;

		return *this;
	}
	PipelineBuilder& PipelineBuilder::set_depth_format(VkFormat format) {
		_renderInfo.depthAttachmentFormat = format;

		return *this;
	}
	PipelineBuilder& PipelineBuilder::disable_depthtest() {
		_depthStencil.depthTestEnable = VK_FALSE;
		_depthStencil.depthWriteEnable = VK_FALSE;
		_depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
		_depthStencil.depthBoundsTestEnable = VK_FALSE;
		_depthStencil.stencilTestEnable = VK_FALSE;
		_depthStencil.front = {};
		_depthStencil.back = {};
		_depthStencil.minDepthBounds = 0.f;
		_depthStencil.maxDepthBounds = 1.f;

		return *this;
	}
	PipelineBuilder& PipelineBuilder::enable_depthtest(bool depthWriteEnable, VkCompareOp op) {
		_depthStencil.depthTestEnable = VK_TRUE;
		_depthStencil.depthWriteEnable = depthWriteEnable;
		_depthStencil.depthCompareOp = op;
		_depthStencil.depthBoundsTestEnable = VK_FALSE;
		_depthStencil.stencilTestEnable = VK_FALSE;
		_depthStencil.front = {};
		_depthStencil.back = {};
		_depthStencil.minDepthBounds = 0.f;
		_depthStencil.maxDepthBounds = 1.f;

		return *this;
	}
}



