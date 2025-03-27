//
// Created by Hayden Rivas on 1/16/25.
//


#include <volk.h>
#include <fmt/core.h>

#include "Slate/VK/vkinfo.h"
#include "Slate/VK/vkutil.h"


namespace Slate::vkutil {
	VkImageAspectFlags AspectMaskFromLayout(VkImageLayout layout) {
		switch (layout) {
			case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL: return VK_IMAGE_ASPECT_DEPTH_BIT;
			case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL: return VK_IMAGE_ASPECT_STENCIL_BIT;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

			default: return VK_IMAGE_ASPECT_COLOR_BIT;
		}
	}
	VkImageAspectFlags AspectMaskFromFormat(VkFormat format) {
		switch (format) {
			case VK_FORMAT_D32_SFLOAT: return VK_IMAGE_ASPECT_DEPTH_BIT;
			case VK_FORMAT_D32_SFLOAT_S8_UINT: return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

			default: return VK_IMAGE_ASPECT_COLOR_BIT;
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
		barrier.subresourceRange = vkinfo::CreateImageSubresourceRange(AspectMaskFromLayout(newLayout));
		barrier.image = image;
		VkDependencyInfo info = { .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .pNext = nullptr };
		info.imageMemoryBarrierCount = 1;
		info.pImageMemoryBarriers = &barrier;
		vkCmdPipelineBarrier2KHR(cmd, &info);
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

		vkCmdBlitImage2KHR(cmd, &blitInfo);
	}

	void SetViewport(VkCommandBuffer cmd, VkExtent2D extent2D) {
		// we flip the viewport because Vulkan is reversed using LH instead of GL's RH
		VkViewport viewport = {};
		viewport.x = 0;
		// using Slang compilier option to reflect the y axis solves this
//		viewport.y = static_cast<float>(extent2D.height);
//		viewport.width = static_cast<float>(extent2D.width);
//		viewport.height = -static_cast<float>(extent2D.height);
		viewport.y = 0;
		viewport.width = static_cast<float>(extent2D.width);
		viewport.height = static_cast<float>(extent2D.height);
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;
		vkCmdSetViewport(cmd, 0, 1, &viewport);
	}
	void SetScissor(VkCommandBuffer cmd, VkExtent2D extent2D, VkOffset2D offset2D) {
		VkRect2D scissor = {};
		scissor.offset.x = offset2D.x; // default 0 for both
		scissor.offset.y = offset2D.y;
		scissor.extent = extent2D;
		vkCmdSetScissor(cmd, 0, 1, &scissor);
	}

	void GenerateMipmaps(VkCommandBuffer cmd, VkImage image, VkExtent2D imageSize) {
		int mipLevels = int(std::floor(std::log2(std::max(imageSize.width, imageSize.height)))) + 1;
		for (int mip = 0; mip < mipLevels; mip++) {

			VkExtent2D halfSize = imageSize;
			halfSize.width /= 2;
			halfSize.height /= 2;

			VkImageMemoryBarrier2 imageBarrier { .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2, .pNext = nullptr };

			imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
			imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

			imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

			VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBarrier.subresourceRange = vkinfo::CreateImageSubresourceRange(aspectMask);
			imageBarrier.subresourceRange.levelCount = 1;
			imageBarrier.subresourceRange.baseMipLevel = mip;
			imageBarrier.image = image;

			VkDependencyInfo depInfo { .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .pNext = nullptr };
			depInfo.imageMemoryBarrierCount = 1;
			depInfo.pImageMemoryBarriers = &imageBarrier;

			vkCmdPipelineBarrier2KHR(cmd, &depInfo);

			if (mip < mipLevels - 1) {
				VkImageBlit2 blitRegion { .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr };

				blitRegion.srcOffsets[1].x = imageSize.width;
				blitRegion.srcOffsets[1].y = imageSize.height;
				blitRegion.srcOffsets[1].z = 1;

				blitRegion.dstOffsets[1].x = halfSize.width;
				blitRegion.dstOffsets[1].y = halfSize.height;
				blitRegion.dstOffsets[1].z = 1;

				blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blitRegion.srcSubresource.baseArrayLayer = 0;
				blitRegion.srcSubresource.layerCount = 1;
				blitRegion.srcSubresource.mipLevel = mip;

				blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blitRegion.dstSubresource.baseArrayLayer = 0;
				blitRegion.dstSubresource.layerCount = 1;
				blitRegion.dstSubresource.mipLevel = mip + 1;

				VkBlitImageInfo2 blitInfo {.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr};
				blitInfo.dstImage = image;
				blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				blitInfo.srcImage = image;
				blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				blitInfo.filter = VK_FILTER_LINEAR;
				blitInfo.regionCount = 1;
				blitInfo.pRegions = &blitRegion;

				vkCmdBlitImage2KHR(cmd, &blitInfo);

				imageSize = halfSize;
			}
		}
		// transition all mip levels into the final read_only layout
		TransitionImageLayout(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
}
