//
// Created by Hayden Rivas on 1/16/25.
//
#pragma once
#include <filesystem>

namespace Slate::vkutil {

	VkImageAspectFlags AspectMaskFromLayout(VkImageLayout layout);
	VkImageAspectFlags AspectMaskFromFormat(VkFormat format);

	void TransitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
	void BlitImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);

	void SetViewport(VkCommandBuffer cmd, VkExtent2D extent2D);
	void SetScissor(VkCommandBuffer cmd, VkExtent2D extent2D, VkOffset2D offset2D = {0, 0});

	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void GenerateMipmaps(VkCommandBuffer cmd, VkImage image, VkExtent2D imageSize);
}// namespace Slate::vkutil