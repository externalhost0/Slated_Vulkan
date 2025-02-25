//
// Created by Hayden Rivas on 1/15/25.
//
#include <cmath>
#include <span>

#include <volk.h>
#include "Slate/VK/vkinfo.h"
#include "Slate/VK/vktypes.h"

namespace Slate::vkinfo {

	VkCommandPoolCreateInfo CreateCommandPoolInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags) {
		VkCommandPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.pNext = nullptr;
		info.queueFamilyIndex = queueFamilyIndex;
		info.flags = flags;
		return info;
	}
	VkCommandBufferAllocateInfo CreateCommandBufferAllocateInfo(VkCommandPool pool, uint32_t count) {
		VkCommandBufferAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.pNext = nullptr;
		info.commandPool = pool;
		info.commandBufferCount = count;
		info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		return info;
	}
	VkFenceCreateInfo CreateFenceInfo(VkFenceCreateFlags flags) {
		VkFenceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = flags;
		return info;
	}
	VkSemaphoreCreateInfo CreateSemaphoreInfo(VkSemaphoreCreateFlags flags) {
		VkSemaphoreCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = flags;
		return info;
	}

	VkRenderingAttachmentInfo CreateColorAttachmentInfo(VkImageView view, VkClearColorValue* clear, VkImageView resolveView, VkResolveModeFlags resolveFlags) {
		VkRenderingAttachmentInfo colorAttachment = { .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
		colorAttachment.pNext = nullptr;

		colorAttachment.imageView = view;
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		colorAttachment.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		if (clear) colorAttachment.clearValue.color = *clear;
		colorAttachment.storeOp = resolveView ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;

		if (resolveView) {
			colorAttachment.resolveImageView = resolveView;
			colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment.resolveMode = static_cast<VkResolveModeFlagBits>(resolveFlags);
		}
		return colorAttachment;
	}
	VkRenderingAttachmentInfo CreateDepthAttachmentInfo(VkImageView view, VkClearDepthStencilValue* clear) {
		VkRenderingAttachmentInfo depthAttachment = {};
		depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		depthAttachment.pNext = nullptr;

		depthAttachment.imageView = view;
		depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		depthAttachment.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		if (clear) depthAttachment.clearValue.depthStencil = *clear;
		return depthAttachment;
	}
	VkRenderingAttachmentInfo CreateDepthStencilAttachmentInfo(VkImageView view, VkClearDepthStencilValue* clear) {
		VkRenderingAttachmentInfo depthAttachment = {};
		depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		depthAttachment.pNext = nullptr;

		depthAttachment.imageView = view;
		depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		if (clear) depthAttachment.clearValue.depthStencil = *clear;
		return depthAttachment;
	}

	VkCommandBufferBeginInfo CreateCommandBufferBeginInfo(VkCommandBufferUsageFlags flags) {
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.pNext = nullptr;
		info.flags = flags;
		return info;
	}
	VkRenderingInfo CreateRenderingInfo(VkExtent2D extent2D, std::span<VkRenderingAttachmentInfo> colorAttachments, VkRenderingAttachmentInfo* depthAttachment) {
		VkRenderingInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.renderArea = {
				.offset = { 0, 0 },
				.extent = extent2D,
		};
		info.layerCount = 1;
		info.viewMask = 0;

		info.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
		info.pColorAttachments = colorAttachments.data();
		info.pDepthAttachment = depthAttachment;
		info.pStencilAttachment = VK_NULL_HANDLE;
		return info;
	}
	VkRenderingInfo CreateRenderingInfo(VkExtent2D extent2D, VkRenderingAttachmentInfo* colorAttachment, VkRenderingAttachmentInfo* depthAttachment) {
		VkRenderingInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.renderArea = {
				.offset = { 0, 0 },
				.extent = extent2D,
		};
		info.layerCount = 1;
		info.viewMask = 0;

		info.colorAttachmentCount = 1;
		info.pColorAttachments = colorAttachment;
		info.pDepthAttachment = depthAttachment;
		info.pStencilAttachment = VK_NULL_HANDLE;
		return info;
	}
	VkImageSubresourceRange CreateImageSubresourceRange(VkImageAspectFlags aspectMask) {
		VkImageSubresourceRange subImage {};
		subImage.aspectMask = aspectMask;
		subImage.baseMipLevel = 0;
		subImage.levelCount = VK_REMAINING_MIP_LEVELS;
		subImage.baseArrayLayer = 0;
		subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;
		return subImage;
	}
	VkSubmitInfo2 CreateSubmitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo) {
		VkSubmitInfo2 info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		info.pNext = nullptr;

		info.waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0 : 1;
		info.pWaitSemaphoreInfos = waitSemaphoreInfo;

		info.signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0 : 1;
		info.pSignalSemaphoreInfos = signalSemaphoreInfo;

		info.commandBufferInfoCount = 1;
		info.pCommandBufferInfos = cmd;

		return info;
	}
	VkSemaphoreSubmitInfo CreateSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore) {
		VkSemaphoreSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		info.pNext = nullptr;
		info.semaphore = semaphore;
		info.stageMask = stageMask;
		info.deviceIndex = 0;
		info.value = 1;
		return info;
	}
	VkCommandBufferSubmitInfo CreateCommandBufferSubmitInfo(VkCommandBuffer cmd) {
		VkCommandBufferSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		info.pNext = nullptr;
		info.commandBuffer = cmd;
		info.deviceMask = 0;
		return info;
	}
	VkPipelineLayoutCreateInfo CreatePipelineLayoutInfo() {
		VkPipelineLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.pNext = nullptr;
		// empty defaults
		info.flags = 0;
		info.pPushConstantRanges = nullptr;
		info.pushConstantRangeCount = 0;
		info.pSetLayouts = nullptr;
		info.setLayoutCount = 0;
		return info;
	}
	VkPipelineLayoutCreateInfo CreatePipelineLayoutInfo(VkPushConstantRange* push_constants, VkDescriptorSetLayout* descriptor_layouts) {
		VkPipelineLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;

		info.pPushConstantRanges = push_constants;
		info.pushConstantRangeCount = 1;

		info.pSetLayouts = descriptor_layouts;
		info.setLayoutCount = 1;
		return info;
	}
	VkPipelineLayoutCreateInfo CreatePipelineLayoutInfo(std::span<VkPushConstantRange> push_constants, std::span<VkDescriptorSetLayout> descriptor_layouts) {
		VkPipelineLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;

		info.pPushConstantRanges = push_constants.data();
		info.pushConstantRangeCount = push_constants.size();
		info.pSetLayouts = descriptor_layouts.data();
		info.setLayoutCount = descriptor_layouts.size();
		return info;
	}
	VkPipelineShaderStageCreateInfo CreatePipelineShaderStageInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule, const char* entry) {
		VkPipelineShaderStageCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		info.pNext = nullptr;

		info.stage = stage;
		info.module = shaderModule;
		info.pName = entry; // entry point default to "main"
		return info;
	}
	VkPipelineDynamicStateCreateInfo CreatePipelineDynamicStateInfo(const VkDynamicState* states, uint32_t statescount, VkPipelineDynamicStateCreateFlags flags) {
		VkPipelineDynamicStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = flags;

		info.pDynamicStates = &states[0];
		info.dynamicStateCount = statescount;
		return info;
	}
	VkImageCreateInfo CreateImageInfo(VkExtent3D extent, VkFormat format, VkImageUsageFlags usage_flags, uint32_t mipmap_levels, VkSampleCountFlags sample_flag) {
		VkImageCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.pNext = nullptr;

		info.format = format;
		info.extent = extent;
		info.samples = static_cast<VkSampleCountFlagBits>(sample_flag); // from flags to bits
		info.usage = usage_flags;

		info.imageType = VK_IMAGE_TYPE_2D;

		info.mipLevels = mipmap_levels; // default to 1

		info.arrayLayers = 1;
		info.tiling = VK_IMAGE_TILING_OPTIMAL; // auto decide best format
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		return info;
	}
	VkImageViewCreateInfo CreateImageViewInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags) {
		// build a image-view for the depth image to use for rendering
		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.pNext = nullptr;

		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.image = image;
		info.format = format;
		info.subresourceRange.baseMipLevel = 0;
		info.subresourceRange.levelCount = 1;
		info.subresourceRange.baseArrayLayer = 0;
		info.subresourceRange.layerCount = 1;
		info.subresourceRange.aspectMask = aspectFlags;

		return info;
	}
}
