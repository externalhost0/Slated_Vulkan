//
// Created by Hayden Rivas on 1/16/25.
//
#pragma once
#include <filesystem>
#include "Slate/Components.h"

namespace Slate::vkutil {
	VkImageAspectFlags AspectMaskFromLayout(VkImageLayout layout);
	VkImageAspectFlags AspectMaskFromFormat(VkFormat format);

	void TransitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);

	VkShaderModule CreateShaderModuleEXT(VkDevice device, const std::filesystem::path& path);
	static VkShaderModule CreateShaderModule(VkDevice device, const std::vector<uint32_t>& code);

	ShaderProgram CreateShaderProgram(VkDevice device, const std::filesystem::path& path);
	void DestroyShaderProgramModules(VkDevice device, const ShaderProgram& program);



	void BlitImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);

	void SetViewport(VkCommandBuffer cmd, VkExtent2D extent2D);
	void SetScissor(VkCommandBuffer cmd, VkExtent2D extent2D, VkOffset2D offset2D = {0, 0});

	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void GenerateMipmaps(VkCommandBuffer cmd, VkImage image, VkExtent2D imageSize);
}// namespace Slate::vkutil