//
// Created by Hayden Rivas on 3/8/25.
//

#include "Slate/ShaderPass.h"
#include "Slate/Debug.h"
#include "Slate/PipelineBuilder.h"
#include "Slate/RenderEngine.h"

namespace Slate {
	ShaderPass RenderEngine::CreateShaderPass(const ShaderResource& resource, PassProperties& properties) const {
		ShaderPass pass = {};

		// first build the pipeline for the pass
		PipelineBuilder builder = {};
		builder.set_module(resource.GetModule());
		builder.set_blending_mode(properties.blendmode);
		builder.set_depthtest(properties.depthmode);
		builder.set_multisampling_mode(properties.samplemode);
		builder.set_cull_mode(properties.cullmode);

		builder.set_topology_mode(properties.topologymode);
		builder.set_polygon_mode(properties.polygonmode);

		builder.set_color_formats(std::span(properties.color_formats));
		builder.set_depth_format(properties.depth_format);

		// create pipeline layout
		// common ds0
		DescriptorLayoutBuilder layoutbuilder = {};
		layoutbuilder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER); // camera
		layoutbuilder.addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER); // lighting (optional)
		pass.set0Layout = layoutbuilder.build(this->_vkDevice, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		layoutbuilder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER); // uniforms
		VkDescriptorSetLayout desclayout2 = layoutbuilder.build(this->_vkDevice, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

		VkDescriptorSetLayout desclayouts[] = { pass.set0Layout, desclayout2 };

		VkPushConstantRange range = {
				.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
				.offset = 0,
				.size = sizeof(GPU::DrawPushConstants)
		};
		VkPipelineLayout temp_layout;
		VkPipelineLayoutCreateInfo pipeline_layout_info = vkinfo::CreatePipelineLayoutInfo(&range, std::span(desclayouts));
		VK_CHECK(vkCreatePipelineLayout(this->_vkDevice, &pipeline_layout_info, nullptr, &temp_layout));

		pass.pipeLayout = temp_layout;
		pass.pipeline = builder.build(this->_vkDevice, pass.pipeLayout);
		return pass;
	}
	void ShaderPass::Destroy(VkDevice device) {
		vkDestroyPipelineLayout(device, this->pipeLayout, nullptr);
		vkDestroyPipeline(device, this->pipeline, nullptr);
	}
}





