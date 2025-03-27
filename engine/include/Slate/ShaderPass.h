//
// Created by Hayden Rivas on 3/8/25.
//

#pragma once

#include "PipelineBuilder.h"
#include "SmartPointers.h"

namespace Slate {
	class ShaderPass {
	public:
		void Destroy(VkDevice device);
		VkPipeline getPipeline() { return this->pipeline; }
		VkPipelineLayout getPipelineLayout() { return this->pipeLayout; }
	private:
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkPipelineLayout pipeLayout = VK_NULL_HANDLE;
	public:
		VkDescriptorSetLayout set0Layout = VK_NULL_HANDLE;

		friend class RenderEngine; // for initialization
	};
}






