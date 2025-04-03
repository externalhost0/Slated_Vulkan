//
// Created by Hayden Rivas on 1/16/25.
//
#pragma once
#include "ShaderPass.h"
#include "FastSTD.h"

#include <span>
#include <vector>
#include <volk.h>

namespace Slate {
	enum class BlendingMode {
		OFF,
		ADDITIVE, // (glow)
		ALPHA_BLEND // (standard)
	};
	enum class DepthMode {
		NONE,
		LESS, // (standard)
		GREATER,
		EQUAL,
		ALWAYS
	};
	enum class CullMode {
		OFF,
		BACK, // (standard)
		FRONT
	};
	enum class PolygonMode {
		FILL, // (standard)
		LINE
	};
	enum class TopologyMode {
		TRIANGLE, // (standard)
		LIST,
		STRIP
	};
	enum class MultisampleMode {
		OFF,
		X2,
		X4,
		X8
	};

	// my associations with vulkans enums
	static std::unordered_map<CullMode, VkCullModeFlagBits> CullModeVkCullModeFlagMap = {
			{CullMode::OFF, VK_CULL_MODE_NONE},
			{CullMode::BACK, VK_CULL_MODE_BACK_BIT},
			{CullMode::FRONT, VK_CULL_MODE_FRONT_BIT}
	};
	static std::unordered_map<PolygonMode, VkPolygonMode> PolygonModeVkPolygonMap = {
			{PolygonMode::FILL, VK_POLYGON_MODE_FILL},
			{PolygonMode::LINE, VK_POLYGON_MODE_LINE}
	};
	static std::unordered_map<TopologyMode, VkPrimitiveTopology> TopologyModeVkPrimitiveTopologyMap = {
			{TopologyMode::TRIANGLE, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST},
			{TopologyMode::LIST, VK_PRIMITIVE_TOPOLOGY_LINE_LIST},
			{TopologyMode::STRIP, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP}
	};
	static std::unordered_map<MultisampleMode, VkSampleCountFlagBits> MultisampleModeVkSampleCountFlagMap = {
			{MultisampleMode::OFF, VK_SAMPLE_COUNT_1_BIT},
			{MultisampleMode::X2, VK_SAMPLE_COUNT_2_BIT},
			{MultisampleMode::X4, VK_SAMPLE_COUNT_4_BIT},
			{MultisampleMode::X8, VK_SAMPLE_COUNT_8_BIT}
	};


	struct PassProperties {
		BlendingMode blendmode;
		DepthMode depthmode;
		TopologyMode topologymode;
		PolygonMode polygonmode;
		MultisampleMode samplemode;
		CullMode cullmode;

		std::span<VkFormat> color_formats;
		VkFormat depth_format;
	};


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

		// props
		PipelineBuilder& set_topology_mode(TopologyMode mode = TopologyMode::TRIANGLE);
		PipelineBuilder& set_polygon_mode(PolygonMode mode = PolygonMode::FILL);
		PipelineBuilder& set_multisampling_mode(MultisampleMode mode = MultisampleMode::OFF);
		PipelineBuilder& set_cull_mode(CullMode mode = CullMode::BACK);
		PipelineBuilder& set_blending_mode(BlendingMode mode = BlendingMode::OFF);
		PipelineBuilder& set_depthtest(DepthMode mode = DepthMode::LESS);

		// formats
		PipelineBuilder& set_color_formats(std::span<VkFormat> formats);
		PipelineBuilder& set_depth_format(VkFormat format);
	private:
		void _setBlendtoOff();
		void _setBlendtoAlphaBlend();
		void _setBlendtoAdditive();
	};
}