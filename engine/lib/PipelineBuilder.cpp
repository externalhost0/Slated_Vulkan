//
// Created by Hayden Rivas on 1/16/25.
//
#include "Slate/Debug.h"
#include "Slate/VK/vkinfo.h"

#include "Slate/PipelineBuilder.h"

namespace Slate {
	bool isIntegarFormat(VkFormat format) {
		return (format == VK_FORMAT_R8_UINT || format == VK_FORMAT_R16_UINT || format == VK_FORMAT_R32_UINT ||
			format == VK_FORMAT_R8G8_UINT || format == VK_FORMAT_R16G16_UINT || format == VK_FORMAT_R32G32_UINT ||
			format == VK_FORMAT_R8G8B8_UINT || format == VK_FORMAT_R16G16B16_UINT || format == VK_FORMAT_R32G32B32_UINT ||
			format == VK_FORMAT_R8G8B8A8_UINT || format == VK_FORMAT_R16G16B16A16_UINT || format == VK_FORMAT_R32G32B32A32_UINT);
	}
	void PipelineBuilder::Clear() {
		// clear all of the structs we need back to 0 with their correct stype
		_inputAssembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };

		_rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };

		_multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };

		_depthStencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };

		_renderInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };

		_colorBlendAttachment = { };
		// vector clears
		_colorAttachmentFormats.clear();
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

		std::vector<VkPipelineColorBlendAttachmentState> blendAttachments(_colorAttachmentFormats.size(), _colorBlendAttachment);
		// disable blending if format doesnt accept it
		for (int i = 0; i < blendAttachments.size(); i++) {
			if (isIntegarFormat(_colorAttachmentFormats[i])) {
				blendAttachments[i].blendEnable = VK_FALSE;
			}
		}
		colorBlending.attachmentCount = blendAttachments.size();
		colorBlending.pAttachments = blendAttachments.data();


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
		this->Clear();
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
	PipelineBuilder& PipelineBuilder::set_multisampling_mode(VkSampleCountFlagBits sampleCount) {
		_multisampling.sampleShadingEnable = VK_FALSE;
		_multisampling.rasterizationSamples = sampleCount;
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
	PipelineBuilder& PipelineBuilder::enable_blending_additive() {
		_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		_colorBlendAttachment.blendEnable = VK_TRUE;
		_colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		_colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		_colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		_colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		_colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		_colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		return *this;
	}
	PipelineBuilder& PipelineBuilder::enable_blending_alphablend() {
		_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		_colorBlendAttachment.blendEnable = VK_TRUE;
		_colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		_colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		_colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		_colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		_colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		_colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		return *this;
	}
	PipelineBuilder& PipelineBuilder::set_color_attachment_format(VkFormat format) {
		_colorAttachmentFormats.push_back(format);

		_renderInfo.colorAttachmentCount = 1;
		_renderInfo.pColorAttachmentFormats = &_colorAttachmentFormats.front();

		return *this;
	}
	PipelineBuilder& PipelineBuilder::set_color_attachment_format(std::span<VkFormat> formats) {
		for (VkFormat format : formats) {
			_colorAttachmentFormats.push_back(format);
		}
		_renderInfo.colorAttachmentCount = formats.size();
		_renderInfo.pColorAttachmentFormats = formats.data();

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



