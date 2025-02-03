//
// Created by Hayden Rivas on 1/16/25.
//
#pragma once
#include <vulkan/vulkan.h>
#include <filesystem>
namespace Slate::vkutil {
	VkImageAspectFlags AspectMaskFromLayout(VkImageLayout layout);
	VkImageAspectFlags AspectMaskFromFormat(VkFormat format);

	void TransitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);

	VkShaderModule CreateShaderModule(const std::filesystem::path& path, VkDevice device);

	void BlitImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);

	void SetViewport(VkCommandBuffer cmd, VkExtent2D extent2D);
	void SetScissor(VkCommandBuffer cmd, VkExtent2D extent2D, VkOffset2D offset2D = {0, 0});

	void CreateBuffer(VkDevice device, VkPhysicalDevice physicalDevice, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
}