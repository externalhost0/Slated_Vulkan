//
// Created by Hayden Rivas on 1/16/25.
//
#pragma once
#include <vector>
#include <span>
#include <vulkan/vulkan.h>

namespace Slate {
	class PipelineBuilder {
	public:
		PipelineBuilder() { Clear(); };
		~PipelineBuilder() = default;

	public:
		std::vector<VkPipelineShaderStageCreateInfo> _shaderStages = {};
		VkPipelineInputAssemblyStateCreateInfo _inputAssembly = {};
		VkPipelineRasterizationStateCreateInfo _rasterizer = {};
		VkPipelineMultisampleStateCreateInfo _multisampling = {};
		VkPipelineDepthStencilStateCreateInfo _depthStencil = {};
		VkPipelineRenderingCreateInfo _renderInfo = {};

		VkPipelineColorBlendAttachmentState _colorBlendAttachment = {};
		std::vector<VkFormat> _colorAttachmentFormats = {};
	public:
		VkPipeline build(VkDevice device, VkPipelineLayout layout);
		void Clear();
	public:
		PipelineBuilder& set_shaders(VkShaderModule vertexShader, VkShaderModule fragmentShader);
		PipelineBuilder& set_input_topology(VkPrimitiveTopology topology);
		PipelineBuilder& set_polygon_mode(VkPolygonMode mode);
		PipelineBuilder& set_cull_mode(VkCullModeFlags cullMode, VkFrontFace frontFace);

		PipelineBuilder&set_multisampling_mode(VkSampleCountFlagBits sampleCount);

		PipelineBuilder& disable_blending();
		PipelineBuilder& enable_blending_additive();
		PipelineBuilder& enable_blending_alphablend();

		PipelineBuilder& set_color_attachment_format(VkFormat format);
		PipelineBuilder& set_color_attachment_format(std::span<VkFormat> formats);

		PipelineBuilder& set_depth_format(VkFormat format);
		PipelineBuilder& disable_depthtest();
		PipelineBuilder& enable_depthtest(bool depthWriteEnable, VkCompareOp op);
	};
}