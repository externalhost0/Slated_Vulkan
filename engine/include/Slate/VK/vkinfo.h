//
// Created by Hayden Rivas on 1/15/25.
//
#pragma once
#include <vulkan/vulkan.h>

namespace Slate::vkinfo {
	VkCommandPoolCreateInfo CreateCommandPoolInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
	VkCommandBufferAllocateInfo CreateCommandBufferAllocateInfo(VkCommandPool pool, uint32_t count);
	VkCommandBufferBeginInfo CreateCommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);
	VkCommandBufferSubmitInfo CreateCommandBufferSubmitInfo(VkCommandBuffer cmd);

	VkFenceCreateInfo CreateFenceInfo(VkFenceCreateFlags flags = 0);
	VkSemaphoreCreateInfo CreateSemaphoreInfo(VkSemaphoreCreateFlags flags = 0);
	VkSemaphoreSubmitInfo CreateSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);

	VkPipelineShaderStageCreateInfo CreatePipelineShaderStageInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule, const char* entry = "main");
	VkPipelineLayoutCreateInfo CreatePipelineLayoutInfo();

	VkRenderingAttachmentInfo CreateAttachmentInfo(VkImageView view, VkClearValue* clear, VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingAttachmentInfo CreateDepthAttachmentInfo(VkImageView view, VkImageLayout layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
	VkRenderingInfo CreateRenderingInfo(VkExtent2D extent2D, VkRenderingAttachmentInfo* attachment, VkRenderingAttachmentInfo* depthAttachment);
	VkSubmitInfo2 CreateSubmitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo);

	VkPipelineDynamicStateCreateInfo CreatePipelineDynamicStateInfo(const VkDynamicState* states, uint32_t statescount, VkPipelineDynamicStateCreateFlags flags = 0);

	VkImageViewCreateInfo CreateImageViewInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
	VkImageCreateInfo CreateImageInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
	VkImageSubresourceRange CreateImageSubresourceRange(VkImageAspectFlags aspectMask);
}