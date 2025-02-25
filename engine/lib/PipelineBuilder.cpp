//
// Created by Hayden Rivas on 1/16/25.
//
#include "Slate/Debug.h"

#include "Slate/PipelineBuilder.h"
#include "Slate/Shader.h"
#include "Slate/VK/vkinfo.h"
#include <volk.h>
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
		// the create info for the pipeline we are building
		VkGraphicsPipelineCreateInfo new_pipeline_info = { .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, .pNext = nullptr };

		// --- shaders --- //
		new_pipeline_info.stageCount = static_cast<uint32_t>(_shaderStages.size());
		new_pipeline_info.pStages = _shaderStages.data();

		// ---- colorblending ---- //
		VkPipelineColorBlendStateCreateInfo colorBlending = { .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, .pNext = nullptr };
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		// copy the set blend mode into our attachments, we than do some conditional modifying
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachments(_colorAttachmentFormats.size(), _colorBlendAttachment);
		// if blending is enabled in one of the helper functions, this catches formats that cant use transparency and disables it
		// disable blending if format doesnt accept it
		for (int i = 0; i < blendAttachments.size(); i++) {
			if (isIntegarFormat(_colorAttachmentFormats[i])) {
				blendAttachments[i].blendEnable = VK_FALSE;
			}
		}
		colorBlending.attachmentCount = blendAttachments.size();
		colorBlending.pAttachments = blendAttachments.data();
		std::fill(std::begin(colorBlending.blendConstants), std::end(colorBlending.blendConstants), 0.f);
		new_pipeline_info.pColorBlendState = &colorBlending;

		// ----- dynamic states ----- //
		VkDynamicState state[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_POLYGON_MODE_EXT };
		VkPipelineDynamicStateCreateInfo dynamicInfo = vkinfo::CreatePipelineDynamicStateInfo(state, 3);
		new_pipeline_info.pDynamicState = &dynamicInfo;

		// ------ vertex input ig doesnt matter ----- //
		VkPipelineVertexInputStateCreateInfo _vertexInputInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, .pNext = nullptr };
		new_pipeline_info.pVertexInputState = &_vertexInputInfo;

		// ------ viewport state ------ //
		VkPipelineViewportStateCreateInfo viewportState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .pNext = nullptr };
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;
		new_pipeline_info.pViewportState = &viewportState;

		// everything else that doesnt need configuring on build()
		new_pipeline_info.pNext = &_renderInfo;
		new_pipeline_info.pInputAssemblyState = &_inputAssembly;
		new_pipeline_info.pRasterizationState = &_rasterizer;
		new_pipeline_info.pMultisampleState = &_multisampling;
		new_pipeline_info.pDepthStencilState = &_depthStencil;
		new_pipeline_info.layout = layout; // just the user given layout

		// -- actual creation -- //
		VkPipeline newPipeline;
		VK_CHECK(vkCreateGraphicsPipelines(device, nullptr, 1, &new_pipeline_info, nullptr, &newPipeline));
		this->Clear(); // clear the entire pipeline struct to reuse the PipelineBuilder
		return newPipeline;
	}
	PipelineBuilder& PipelineBuilder::set_program(const ShaderProgram& program) {
		_shaderStages.clear();

		_shaderStages.push_back(vkinfo::CreatePipelineShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT, program.vertModule, "vs_main"));
		_shaderStages.push_back(vkinfo::CreatePipelineShaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT, program.fragModule, "fs_main"));

		return *this;
	}
	PipelineBuilder& PipelineBuilder::set_shadersEXT(VkShaderModule vertexShader, VkShaderModule fragmentShader) {
		_shaderStages.clear();

		_shaderStages.push_back(vkinfo::CreatePipelineShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT, vertexShader));
		_shaderStages.push_back(vkinfo::CreatePipelineShaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader));

		return *this;
	}
	PipelineBuilder& PipelineBuilder::set_default_polygon_mode(VkPolygonMode mode) {
		_rasterizer.polygonMode = mode;
		_rasterizer.lineWidth = 1.0f; // we cant change width, metal doesnt support wide lines

		return *this;
	}
	PipelineBuilder& PipelineBuilder::set_default_topology_mode(VkPrimitiveTopology topology) {
		_inputAssembly.topology = topology;
		_inputAssembly.primitiveRestartEnable = VK_TRUE;
		if (topology == VK_PRIMITIVE_TOPOLOGY_POINT_LIST 				||
			topology == VK_PRIMITIVE_TOPOLOGY_LINE_LIST 				||
			topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST 			||
			topology == VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY  ||
			topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY) _inputAssembly.primitiveRestartEnable = VK_FALSE;

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

		_colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE; // diff

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

		_colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // diff
		// ok yeah i have no idea what the point of one minus is, figure out later
		// ok so that doesnt fix it, turns out you need to multiply rgb and than invert that for alpha
		// TODO: leaving this to tell myself to do better research into alpha operations and shaders

		_colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		_colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		_colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		_colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		return *this;
	}
	PipelineBuilder& PipelineBuilder::enable_blending_premultiplied() {
		_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		_colorBlendAttachment.blendEnable = VK_TRUE;
		_colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;

		_colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // diff

		_colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		_colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		_colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		_colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		return *this;
	}
	PipelineBuilder& PipelineBuilder::set_color_attachment_format(VkFormat format) {
		_colorAttachmentFormats.push_back(format);

		_renderInfo.colorAttachmentCount = 1;
		_renderInfo.pColorAttachmentFormats = &_colorAttachmentFormats.front();

		return *this;
	}
	// for multiple formats
	PipelineBuilder& PipelineBuilder::set_color_attachment_format(std::span<VkFormat> formats) {
		for (const VkFormat& format : formats) {
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
		// diffs
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
		// diffs
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



