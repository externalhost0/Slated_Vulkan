//
// Created by Hayden Rivas on 1/16/25.
//
#pragma once
#include <filesystem>

#include "Slate/Common/Debug.h"
namespace Slate::vkutil {
	struct StageAccess
	{
		VkPipelineStageFlags2 stage;
		VkAccessFlags2 access;
	};
	uint32_t GetTextureBytesPerPlane(uint32_t width, uint32_t height, VkFormat format, uint32_t plane);
	uint32_t GetTextureBytesPerLayer(uint32_t width, uint32_t height, VkFormat format, uint32_t level);
	uint32_t GetBytesPerPixel(VkFormat format);
	uint32_t GetNumImagePlanes(VkFormat format);
	VkExtent2D GetImagePlaneExtent(VkExtent2D plane0, VkFormat format, uint32_t plane);

	uint32_t GetAlignedSize(uint32_t value, uint32_t alignment);

	VkImageAspectFlags AspectMaskFromAttachmentLayout(VkImageLayout layout);
	VkImageAspectFlags AspectMaskFromFormat(VkFormat format);

	bool IsFormatDepthOrStencil(VkFormat format);

	StageAccess getPipelineStageAccess(VkImageLayout layout);
	void ImageMemoryBarrier2(VkCommandBuffer cmd, VkImage image, StageAccess src, StageAccess dst, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange range);

	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);


}// namespace Slate::vkutil