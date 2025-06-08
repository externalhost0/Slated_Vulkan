//
// Created by Hayden Rivas on 4/27/25.
//

#pragma once

#include <volk.h>
#include <vector>

#include "Slate/Common/Invalids.h"
#include "Slate/Common/Handles.h"

namespace Slate {
	// forward declarations
	class GX;
	class AllocatedImage;

	// lets try to abide RAII!
	class VulkanSwapchain final {
		enum { kMAX_SWAPCHAIN_IMAGES = 16 };
	public:
		explicit VulkanSwapchain(GX& gx, uint16_t width = 0, uint16_t height = 0);
		~VulkanSwapchain();
	public:
		InternalTextureHandle acquireCurrentImage();
		void present();

		VkImage getCurrentImage() const;
		VkImageView getCurrentImageView() const;
		VkImageLayout getCurrentImageLayout() const;

		inline uint32_t getNumOfSwapchainImages() const { return _numSwapchainImages; }
		inline bool isDirty() const { return _isDirty; }

		VkImageLayout _vkCurrentImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	private:
		// helper
		AllocatedImage& _getCurrentTexture() const;
		VkSwapchainKHR _vkSwapchain = VK_NULL_HANDLE;
		VkExtent2D _vkExtent2D = {};
		VkFormat _vkImageFormat = VK_FORMAT_UNDEFINED;

		uint32_t _currentImageIndex = 0;
		uint32_t _currentFrameNum = 0;
		uint32_t _numSwapchainImages = 0;
		bool _isDirty = false;
		bool _getNextImage = true;

		InternalTextureHandle _swapchainTextures[kMAX_SWAPCHAIN_IMAGES] = {};
		uint64_t _timelineWaitValues[kMAX_SWAPCHAIN_IMAGES] = {}; // this HERE NEEDS FIXING
		VkSemaphore _vkAcquireSemaphores[kMAX_SWAPCHAIN_IMAGES] = {};

		// injected info
		GX& _gxCtx;
		friend class GX; // for present
	};

}