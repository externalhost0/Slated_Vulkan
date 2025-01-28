//
// Created by Hayden Rivas on 1/16/25.
//

#include "Slate/Files.h"
#include "Slate/VK/vkext.h"
#include "Slate/VK/vkinfo.h"

#include "Slate/VK/vkutil.h"

namespace Slate::vkutil {
	VkImageAspectFlags AspectMaskFromLayout(VkImageLayout layout) {
		switch (layout) {
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: return VK_IMAGE_ASPECT_COLOR_BIT;
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return VK_IMAGE_ASPECT_COLOR_BIT;

			case VK_IMAGE_LAYOUT_GENERAL: return VK_IMAGE_ASPECT_COLOR_BIT;
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return VK_IMAGE_ASPECT_COLOR_BIT;

			case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL: return VK_IMAGE_ASPECT_DEPTH_BIT;
			case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL: return VK_IMAGE_ASPECT_STENCIL_BIT;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			default: return VK_IMAGE_ASPECT_NONE;
		}
	}
	void TransitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout) {
		VkImageMemoryBarrier2 barrier = { .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2, .pNext = nullptr };
		barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
		barrier.oldLayout = currentLayout;
		barrier.newLayout = newLayout;
		VkImageAspectFlags aspectMask = AspectMaskFromLayout(newLayout);
		barrier.subresourceRange = vkinfo::CreateImageSubresourceRange(aspectMask);
		barrier.image = image;
		VkDependencyInfo info = { .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .pNext = nullptr };
		info.imageMemoryBarrierCount = 1;
		info.pImageMemoryBarriers = &barrier;
		vkext::vkCmdPipelineBarrier2(cmd, &info);
	}

	VkShaderModule CreateShaderModule(const std::filesystem::path& path, VkDevice device) {
		std::vector<std::byte> result = ReadBinaryFile(path);

		VkShaderModuleCreateInfo create_info = { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		create_info.codeSize = result.size();
		create_info.pCode = reinterpret_cast<const uint32_t*>(result.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &create_info, nullptr, &shaderModule) != VK_SUCCESS) {
			return VK_NULL_HANDLE;
		}
		return shaderModule;
	}

	void BlitImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize) {
		VkImageBlit2 blitRegion = { .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr };

		blitRegion.srcOffsets[1].x = static_cast<int32_t>(srcSize.width);
		blitRegion.srcOffsets[1].y = static_cast<int32_t>(srcSize.height);
		blitRegion.srcOffsets[1].z = 1;

		blitRegion.dstOffsets[1].x = static_cast<int32_t>(dstSize.width);
		blitRegion.dstOffsets[1].y = static_cast<int32_t>(dstSize.height);
		blitRegion.dstOffsets[1].z = 1;

		blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.srcSubresource.baseArrayLayer = 0;
		blitRegion.srcSubresource.layerCount = 1;
		blitRegion.srcSubresource.mipLevel = 0;

		blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.dstSubresource.baseArrayLayer = 0;
		blitRegion.dstSubresource.layerCount = 1;
		blitRegion.dstSubresource.mipLevel = 0;

		VkBlitImageInfo2 blitInfo{ .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr };
		blitInfo.dstImage = destination;
		blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		blitInfo.srcImage = source;
		blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		blitInfo.filter = VK_FILTER_LINEAR;
		blitInfo.regionCount = 1;
		blitInfo.pRegions = &blitRegion;

		vkext::vkCmdBlitImage2(cmd, &blitInfo);
	}

	void SetViewport(VkCommandBuffer cmd, VkExtent2D extent2D) {
		// we flip the viewport because Vulkan is reversed using LH instead of GL's RH
		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = static_cast<float>(extent2D.height);
		viewport.width = static_cast<float>(extent2D.width);
		viewport.height = -static_cast<float>(extent2D.height);
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;
		vkCmdSetViewport(cmd, 0, 1, &viewport);
	}
	void SetScissor(VkCommandBuffer cmd, VkExtent2D extent2D) {
		VkRect2D scissor = {};
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent = extent2D;
		vkCmdSetScissor(cmd, 0, 1, &scissor);
	}

}
