//
// Created by Hayden Rivas on 1/16/25.
//
#pragma once
#include <vulkan/vulkan.h>
#include <filesystem>
namespace Slate::vkutil {
	void TransitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);

	VkShaderModule CreateShaderModule(const std::filesystem::path& path, VkDevice device);

	void BlitImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);

	void SetViewport(VkCommandBuffer cmd, VkExtent2D extent2D);
	void SetScissor(VkCommandBuffer cmd, VkExtent2D extent2D);

}// namespace Slate::vkutil