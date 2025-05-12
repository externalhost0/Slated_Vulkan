//
// Created by Hayden Rivas on 1/16/25.
//
#pragma once
#include "Slate/Common/FastSTD.h"
#include "Slate/VK/vkenums.h"

#include <span>
#include <vector>
#include <volk.h>

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

		FastVector<VkPipelineShaderStageCreateInfo, 2> _shaderStages = {};
		FastVector<VkFormat, 12> _colorAttachmentFormats = {};
	public:
		VkPipeline build(VkDevice device, VkPipelineLayout layout);
		void Clear();
	public:
		// program
		PipelineBuilder& set_module(const VkShaderModule& module);
		PipelineBuilder& set_moduleEXT(const VkShaderModule& vertModule, const VkShaderModule& fragModule);

		// props
		PipelineBuilder& set_topology_mode(TopologyMode mode);
		PipelineBuilder& set_polygon_mode(PolygonMode mode);
		PipelineBuilder& set_multisampling_mode(SampleCount count);
		PipelineBuilder& set_cull_mode(CullMode mode);
		PipelineBuilder& set_blending_mode(BlendingMode mode);
		PipelineBuilder& set_depthtest(DepthMode mode);

		// formats
		PipelineBuilder& set_color_formats(std::span<VkFormat> formats);
		PipelineBuilder& set_depth_format(VkFormat format);
	private:
		void _setBlendtoOff();
		void _setBlendtoAlphaBlend();
		void _setBlendtoAdditive();
	};
}