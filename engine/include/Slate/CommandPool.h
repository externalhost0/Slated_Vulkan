//
// Created by Hayden Rivas on 4/23/25.
//

#pragma once

#include <volk.h>
namespace Slate {
	class CommandBuffer;

	class CommandPool final {
	public:
		void create(VkDevice device, uint32_t queueIndex);
		void destroy(VkDevice device);
	public:
		VkResult reset(VkDevice device);
		void allocateCmdBuffer(VkDevice device, CommandBuffer& cmdBuffer);
		void freeCmdBuffer(VkDevice device, CommandBuffer& cmdBuffer);
	private:
		VkDevice* _device = nullptr;
		VkCommandPool _vkCommandPool = VK_NULL_HANDLE;
	};
}