//
// Created by Hayden Rivas on 1/16/25.
//
#pragma once
#include <span>
#include <vector>
#include <volk.h>
#include "Shader.h"

namespace Slate {
	class PipelineBuilder {
	public:
		PipelineBuilder() { Clear(); };
		~PipelineBuilder() = default;

	public:
		VkPipelineInputAssemblyStateCreateInfo _inputAssembly = {};
		VkPipelineRasterizationStateCreateInfo _rasterizer = {};
		VkPipelineMultisampleStateCreateInfo _multisampling = {};
		VkPipelineDepthStencilStateCreateInfo _depthStencil = {};
		VkPipelineRenderingCreateInfo _renderInfo = {};
		VkPipelineColorBlendAttachmentState _colorBlendAttachment = {};

		std::vector<VkPipelineShaderStageCreateInfo> _shaderStages = {};
		std::vector<VkFormat> _colorAttachmentFormats = {};
	public:
		VkPipeline build(VkDevice device, VkPipelineLayout layout);
		void Clear();
	public:
		PipelineBuilder& set_program(const ShaderProgram& program);
		PipelineBuilder& set_shadersEXT(VkShaderModule vertexShader, VkShaderModule fragmentShader);
		PipelineBuilder& set_default_topology_mode(VkPrimitiveTopology topology);
		PipelineBuilder& set_default_polygon_mode(VkPolygonMode mode);
		PipelineBuilder& set_cull_mode(VkCullModeFlags cullMode, VkFrontFace frontFace);

		PipelineBuilder& set_multisampling_mode(VkSampleCountFlagBits sampleCount);

		PipelineBuilder& disable_blending();
		PipelineBuilder& enable_blending_additive();
		PipelineBuilder& enable_blending_alphablend();
		PipelineBuilder& enable_blending_premultiplied();

		PipelineBuilder& set_color_attachment_format(VkFormat format);
		PipelineBuilder& set_color_attachment_format(std::span<VkFormat> formats);

		PipelineBuilder& set_depth_format(VkFormat format);
		PipelineBuilder& disable_depthtest();
		PipelineBuilder& enable_depthtest(bool depthWriteEnable, VkCompareOp op);
	};
}