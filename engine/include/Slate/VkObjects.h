//
// Created by Hayden Rivas on 4/15/25.
//

#pragma once
#include <slang/slang-com-ptr.h>
#include <slang/slang.h>
#include <vk_mem_alloc.h>
#include <volk.h>


namespace Slate {
	// forward declare
	class GX;

	struct AllocatedSampler final {
		VkSampler _vkSampler;
		char debugName[256] = {0};
	};

	struct AllocatedImage final {
		void generateMipmap(VkCommandBuffer cmd);
		void transitionLayout(VkCommandBuffer cmd, VkImageLayout newImageLayout, const VkImageSubresourceRange& subresourceRange);

		[[nodiscard]] inline bool isSampledImage() const { return (_vkUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) > 0; }
		[[nodiscard]] inline bool isStorageImage() const { return (_vkUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT) > 0; }
		[[nodiscard]] inline bool isColorAttachment() const { return (_vkUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) > 0; }
		[[nodiscard]] inline bool isDepthAttachment() const { return (_vkUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) > 0; }
		[[nodiscard]] inline bool isAttachment() const { return (_vkUsageFlags & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) > 0; }
		[[nodiscard]] inline bool isSwapchainImage() const { return _isSwapchainImage; }

		[[nodiscard]] inline VkSampleCountFlagBits getSampleCount() const { return _vkSampleCountFlagBits; }
		[[nodiscard]] inline VkImageType getType() const { return _vkImageType; }
		[[nodiscard]] inline VkImageLayout getLayout() const { return _vkCurrentImageLayout; }
	private:
		VkImage _vkImage = VK_NULL_HANDLE;
		VkImageView _vkImageView = VK_NULL_HANDLE;
		VkImageView _vkImageViewStorage = VK_NULL_HANDLE;

		VkExtent3D _vkExtent = {};
		VkFormat _vkFormat = VK_FORMAT_UNDEFINED;
		VkFormatProperties _vkFormatProperties = {};
		uint32_t _numLevels = 1u;
		uint32_t _numLayers = 1u;

		VkImageLayout _vkCurrentImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageType _vkImageType = VK_IMAGE_TYPE_MAX_ENUM;
		VkImageUsageFlags _vkUsageFlags = 0;
		VkSampleCountFlagBits _vkSampleCountFlagBits = VK_SAMPLE_COUNT_1_BIT;

		VmaAllocation _vmaAllocation = nullptr;

		void* _mappedPtr = nullptr;
		bool _isResolveAttachment = false;
		bool _isSwapchainImage = false;
		bool _isOwning = true;
		char _debugName[256] = {0};

		friend class GX;
		friend class GXBackend;
		friend class VulkanActions;
		friend class CommandBuffer;
		friend class VulkanSwapchain;
		friend class VulkanStagingDevice;
	};

	struct AllocatedBuffer final {
		VkBuffer _vkBuffer = VK_NULL_HANDLE;

	public:
		[[nodiscard]] inline bool isMapped() const { return _mappedPtr != nullptr; }
		[[nodiscard]] inline uint8_t* getMappedPtr() const { return static_cast<uint8_t*>(_mappedPtr); }

		void bufferSubData(const GX& ctx, size_t offset, size_t size, const void* data);
		void getBufferSubData(const GX& ctx, size_t offset, size_t size, void* data);
		void flushMappedMemory(const GX& ctx, VkDeviceSize offset, VkDeviceSize size) const;
		void invalidateMappedMemory(const GX& ctx, VkDeviceSize offset, VkDeviceSize size) const;
	private:
		VkDeviceMemory _vkMemory = VK_NULL_HANDLE;
		VmaAllocation _vmaAllocation = VK_NULL_HANDLE;

		VkDeviceSize _bufferSize = 0;
		VkBufferUsageFlags _vkUsageFlags = 0;
		VkMemoryPropertyFlags _vkMemoryPropertyFlags = 0;
		VkDeviceAddress _vkDeviceAddress = 0; // optional, for shader access

		void* _mappedPtr = nullptr;
		bool _isCoherentMemory = false;
		char _debugName[256] = {0};

		friend class GX;
		friend class GXBackend;
		friend class VulkanActions;
		friend class CommandBuffer;
		friend class VulkanStagingDevice;
	};
}