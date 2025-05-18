//
// Created by Hayden Rivas on 4/27/25.
//
#include <VkBootstrap.h>

#include "Slate/Common/Logger.h"
#include "Slate/GX.h"
#include "Slate/VK/vkinfo.h"
#include "Slate/VulkanSwapchain.h"

namespace Slate {
	// FIXME: potentially dangerous
	AllocatedImage& VulkanSwapchain::_getCurrentTexture() const {
		return *_gxCtx._texturePool.get(_swapchainTextures[_currentImageIndex]);
	}
	VkImage VulkanSwapchain::getCurrentImage() const {
			return _getCurrentTexture()._vkImage;
	}
	VkImageView VulkanSwapchain::getCurrentImageView() const {
			return _getCurrentTexture()._vkImageView;
	}
	VkImageLayout VulkanSwapchain::getCurrentImageLayout() const {
			return _getCurrentTexture()._vkCurrentImageLayout;
	}

	VulkanSwapchain::VulkanSwapchain(GX& gx, uint16_t width, uint16_t height) : _gxCtx(gx) {
		ASSERT(_gxCtx._backend.getSurface());
		// PRIMARY SWAPCHAIN DATA CREATION //
		{
			vkb::SwapchainBuilder swapchainBuilder{gx._backend.getPhysicalDevice(), gx._backend.getDevice(), gx._backend.getSurface()};
			auto swapchain_result = swapchainBuilder
											.set_desired_format(VkSurfaceFormatKHR{
													.format = VkFormat::VK_FORMAT_B8G8R8A8_UNORM,
													.colorSpace = VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
											.set_desired_present_mode(VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR)// VERY IMPORTANT, decides framerate/buffer/sync
											.set_desired_extent(width, height)
											.add_image_usage_flags(VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT)
											.build();
			ASSERT_MSG(swapchain_result.has_value(), "[VULKAN] {}", swapchain_result.error().message().c_str());

			vkb::Swapchain& vkbswapchain = swapchain_result.value();
			// things we assign to the class itself
			this->_vkSwapchain = vkbswapchain.swapchain;
			this->_vkExtent2D = vkbswapchain.extent;
			this->_vkImageFormat = vkbswapchain.image_format;
			if (vkbswapchain.image_count > kMAX_SWAPCHAIN_IMAGES) {
				LOG_USER(LogType::FatalError, "Swapchain image count ({}) exceeds max supported swapchain images ({})!", (int) vkbswapchain.image_count, (int) kMAX_SWAPCHAIN_IMAGES);
			}
			this->_numSwapchainImages = vkbswapchain.image_count;
			// things we assign to the textures we will craete
			std::vector<VkImage> images = vkbswapchain.get_images().value();
			std::vector<VkImageView> imageviews = vkbswapchain.get_image_views().value();
			// now for the number of swapchain images available, create a texture handle for it
			VkImageUsageFlags usage_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			{
				VkSurfaceCapabilitiesKHR caps = {};
				VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gx._backend.getPhysicalDevice(), gx._backend.getSurface(), &caps));

				VkFormatProperties props = {};
				vkGetPhysicalDeviceFormatProperties(gx._backend.getPhysicalDevice(), _vkImageFormat, &props);

				const bool isStorageSupported = (caps.supportedUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT) > 0;
				const bool isTilingOptimalSupported = (props.optimalTilingFeatures & VK_IMAGE_USAGE_STORAGE_BIT) > 0;

				if (isStorageSupported && isTilingOptimalSupported) {
					usage_flags |= VK_IMAGE_USAGE_STORAGE_BIT;
				}
			}
			for (uint32_t i = 0; i < _numSwapchainImages; i++) {
				// give necessary info to texture
				AllocatedImage image = {};
				image._isSwapchainImage = true,
				image._isOwning = false;
				image._vkUsageFlags = usage_flags;
				image._vkImageType = VK_IMAGE_TYPE_2D;
				image._vkImage = std::move(images[i]),
				image._vkImageView = std::move(imageviews[i]),
				image._vkCurrentImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				image._vkExtent = { _vkExtent2D.width, _vkExtent2D.height, 1},
				image._vkFormat = _vkImageFormat;
				snprintf(image._debugName, sizeof(image._debugName), "Swapchain Image %d", i);
				this->_swapchainTextures[i] = _gxCtx._texturePool.create(std::move(image));
			}
		}
		// SECONDARY DATA CREATION //
		{
			VkSemaphoreCreateInfo semaphoreInfo = vkinfo::CreateSemaphoreInfo();
			for (int i = 0; i < _numSwapchainImages; i++) {
				VK_CHECK(vkCreateSemaphore(_gxCtx._backend.getDevice(), &semaphoreInfo, nullptr, &_vkAcquireSemaphores[i]));
			}
		}
	}

	VulkanSwapchain::~VulkanSwapchain() {
		// DESTROY MAIN SWAPCHAIN DATA //
		{
			for (int i = 0; i < _numSwapchainImages; i++) {
				_gxCtx.destroy(_swapchainTextures[i]);
			}
			// images are destroyed alongside swapchain destruction
			vkDestroySwapchainKHR(_gxCtx._backend.getDevice(), _vkSwapchain, nullptr);
		}
		// DESTROY SECONDARY DATA //
		{
			for (VkSemaphore semaphore : _vkAcquireSemaphores) {
				vkDestroySemaphore(_gxCtx._backend.getDevice(), semaphore, nullptr);
			}
		}
	}

	TextureHandle VulkanSwapchain::acquireCurrentImage() {
		if (_getNextImage) {
			const VkSemaphoreWaitInfo waitInfo = {
					.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
					.semaphoreCount = 1,
					.pSemaphores = &_gxCtx._timelineSemaphore,
					.pValues = &_timelineWaitValues[_currentImageIndex],
			};
			ASSERT_MSG(_currentImageIndex < (sizeof(_timelineWaitValues)/sizeof(_timelineWaitValues[0])), "Image index out of range");
			VK_CHECK(vkWaitSemaphores(_gxCtx._backend.getDevice(), &waitInfo, UINT64_MAX));
			VkSemaphore acquireSemaphore = _vkAcquireSemaphores[_currentImageIndex];
			VkResult r = vkAcquireNextImageKHR(_gxCtx._backend.getDevice(), _vkSwapchain, UINT64_MAX, acquireSemaphore, nullptr, &_currentImageIndex);
			if (r != VK_SUCCESS && r != VK_SUBOPTIMAL_KHR && r != VK_ERROR_OUT_OF_DATE_KHR) {
				ASSERT(r);
			}
			_getNextImage = false;
			_gxCtx._imm->waitSemaphore(acquireSemaphore);
		}

		if (_currentImageIndex < _numSwapchainImages) {
			return _swapchainTextures[_currentImageIndex];
		}
		return {};
	}

	void VulkanSwapchain::present() {
		ASSERT_MSG(_vkCurrentImageLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, "Swapchain image layout is not VK_IMAGE_LAYOUT_PRESENT_SRC_KHR!");
		VkSemaphore semaphore = _gxCtx._imm->acquireLastSubmitSemaphore();

		const VkPresentInfoKHR present_info = {
				.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
				.pNext = nullptr,
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = &semaphore,
				.swapchainCount = 1u,
				.pSwapchains = &_vkSwapchain,
				.pImageIndices = &_currentImageIndex,
		};
		VkResult presentResult = vkQueuePresentKHR(_gxCtx._backend.getGraphicsQueue(), &present_info);
		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || _isDirty) {
			_isDirty = true;
			if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) return;
		}
		_getNextImage = true;
		_currentFrameNum++;
	}

}


