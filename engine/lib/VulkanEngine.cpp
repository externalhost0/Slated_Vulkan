//
// Created by Hayden Rivas on 1/14/25.
//
// external headers
#define VOLK_IMPLEMENTATION
#include <volk.h>

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_IMPLEMENTATION
#define VMA_LEAK_LOG_FORMAT(format, ...) {  \
    printf((format), __VA_ARGS__);          \
    printf("\n");           			    \
}
#include <vk_mem_alloc.h>
#include <VkBootstrap.h>
// shader reflection headers
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


// my headers
#include "Slate/Debug.h"
#include "Slate/Filesystem.h"
#include "Slate/VK/vkinfo.h"
#include "Slate/VK/vkutil.h"
#include "Slate/VulkanEngine.h"

namespace Slate {
	void VulkanEngine::CreateInstance(const VulkanInstanceInfo &info) {
		EXPECT(volkInitialize() == VK_SUCCESS, "Volk failed to initialize!"); // kinda important

		vkb::InstanceBuilder builder;
		auto instance_result = builder
									   .set_app_name(info.app_name)
									   .set_app_version(info.app_version.major, info.app_version.minor, info.app_version.patch)
									   .set_engine_name(info.engine_name)
									   .set_engine_version(info.engine_version.major, info.engine_version.minor, info.engine_version.patch)

									   .request_validation_layers()
									   .use_default_debug_messenger()
									   .require_api_version(VK_API_VERSION_1_3)
									   .build();
		if (!instance_result) {
			EXPECT(false, "Failed to create Vulkan instance. Error: {}", instance_result.error().message().c_str())
		}
		vkb::Instance& vkbinstance = instance_result.value();
		_bootstrap.VkbInstance = vkbinstance;
		_vkInstance = vkbinstance.instance;
		_vkDebugMessenger = vkbinstance.debug_messenger;

		volkLoadInstance(_vkInstance);
	}
	void VulkanEngine::CreateSurface(GLFWwindow *pGlfwWindow) {
		// surface creation
		if (glfwCreateWindowSurface(_vkInstance, pGlfwWindow, nullptr, &_vkSurfaceKHR) != VK_SUCCESS) {
			EXPECT(false, "Vulkan & GLFW surface creation failed!")
		}
	}
	void VulkanEngine::CreateDevices() {
		// GET PHYSICAL DEVICE
		//vulkan 1.3 features
		VkPhysicalDeviceVulkan13Features features13 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		features13.synchronization2 = true;
		features13.dynamicRendering = true;

		//vulkan 1.2 features
		VkPhysicalDeviceVulkan12Features features12 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		features12.descriptorIndexing = true;
		features12.bufferDeviceAddress = true;

		// vulkan 1.1 features
		VkPhysicalDeviceVulkan11Features features11 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };

		//vulkan 1.0 features
		VkPhysicalDeviceFeatures features = {};
		features.fragmentStoresAndAtomics = true;
		features.fillModeNonSolid = true;
		features.independentBlend = true;
		features.shaderInt64 = true;

		vkb::PhysicalDeviceSelector selector(_bootstrap.VkbInstance);
		auto physical_result = selector
									   .set_minimum_version(1, 2)
									   .add_required_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
									   .add_required_extension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)
									   .add_required_extension(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME)
									   .add_required_extension(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME)
									   .add_required_extension(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME)
									   .add_required_extension(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME)
									   //  .set_required_features_13(features13) //cant be on until 1.3 mvk
									   .set_required_features_12(features12)
									   .set_required_features_11(features11)
									   .set_required_features(features)
									   .set_surface(_vkSurfaceKHR)
									   .select();
		if (!physical_result) {
			EXPECT(false, "Failed to select Vulkan Physical Device. Error: {}", physical_result.error().message().c_str())
		}
		vkb::PhysicalDevice& vkbphysdevice = physical_result.value();
		_bootstrap.VkbPhysicalDevice = vkbphysdevice;
		_vkPhysicalDevice = vkbphysdevice.physical_device;


		// i have no idea why this is a physica device feature but im adding it into the logical device builder
		// ok so basically to not set off the validation layer, we need to use this on the physical device, we can uncomment required features 13 when 1.3 is ready for moltenvk
		VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_feature = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR };
		dynamic_rendering_feature.pNext = nullptr;
		dynamic_rendering_feature.dynamicRendering = VK_TRUE;

		VkPhysicalDeviceSynchronization2FeaturesKHR synchronization2_feature = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR };
		synchronization2_feature.pNext = nullptr;
		synchronization2_feature.synchronization2 = VK_TRUE;

		VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynamic_state_feature = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT };
		dynamic_state_feature.pNext = nullptr;
		dynamic_state_feature.extendedDynamicState = VK_TRUE;

		VkPhysicalDeviceExtendedDynamicState2FeaturesEXT dynamic_state2_feature = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT };
		dynamic_state2_feature.pNext = nullptr;
		dynamic_state2_feature.extendedDynamicState2 = VK_TRUE;

		VkPhysicalDeviceExtendedDynamicState3FeaturesEXT dynamic_state3_feature = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT };
		dynamic_state3_feature.pNext = nullptr;
		dynamic_state3_feature.extendedDynamicState3PolygonMode = VK_TRUE;

		// GET DEVICE
		vkb::DeviceBuilder device_builder{_bootstrap.VkbPhysicalDevice};
		auto device_result = device_builder
									 .add_pNext(&dynamic_rendering_feature)
									 .add_pNext(&synchronization2_feature)
									 .add_pNext(&dynamic_state_feature)
									 .add_pNext(&dynamic_state2_feature)
									 .add_pNext(&dynamic_state3_feature)
									 .build();
		if (!device_result) {
			EXPECT(false, "Failed to create Vulkan device. Error: {}", device_result.error().message().c_str())
		}
		vkb::Device& vkbdevice = device_result.value();
		_bootstrap.VkbDevice = vkbdevice;
		_vkDevice = vkbdevice.device;

		volkLoadDevice(_vkDevice);
	}
	void VulkanEngine::CreateAllocator() { // done after device creation
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = _vkPhysicalDevice;
		allocatorInfo.device = _vkDevice;
		allocatorInfo.instance = _vkInstance;
		allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
		const VmaVulkanFunctions functions = {
				.vkGetInstanceProcAddr = vkGetInstanceProcAddr,
				.vkGetDeviceProcAddr = vkGetDeviceProcAddr
		};
		allocatorInfo.pVulkanFunctions = &functions;
		vmaCreateAllocator(&allocatorInfo, &_allocator);
	}
	void VulkanEngine::CreateQueues() {
		// GETTING QUEUES!

		// graphics queue
		auto graphics_queue_result = _bootstrap.VkbDevice.get_queue(vkb::QueueType::graphics);
		if (!graphics_queue_result) {
			EXPECT(false, "Failed to get graphics queue. Error: {}", graphics_queue_result.error().message().c_str())
		}
		_vkGraphicsQueue = graphics_queue_result.value();
		// graphics queue index
		auto graphics_queue_index_result = _bootstrap.VkbDevice.get_queue_index(vkb::QueueType::graphics);
		if (!graphics_queue_index_result) {
			EXPECT(false, "Failed to get graphics queue index/family. Error: {}", graphics_queue_index_result.error().message().c_str())
		}
		_vkGraphicsQueueFamily = graphics_queue_index_result.value();

		// present queue
		auto present_queue_result = _bootstrap.VkbDevice.get_queue(vkb::QueueType::present);
		if (!present_queue_result) {
			EXPECT(false, "Failed to get present queue. Error: {}", present_queue_result.error().message().c_str())
		}
		_vkPresentQueue = present_queue_result.value();
	}

	void VulkanEngine::CreateSwapchain(uint16_t width, uint16_t height) {
		vkb::SwapchainBuilder swapchainBuilder(_vkPhysicalDevice, _vkDevice, _vkSurfaceKHR);
		auto swapchain_result = swapchainBuilder
										.set_desired_format(VkSurfaceFormatKHR{
												.format = VkFormat::VK_FORMAT_B8G8R8A8_UNORM,
												.colorSpace = VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
										.set_desired_present_mode(VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR)// VERY IMPORTANT, decides framerate/buffer/sync
										.set_desired_extent(width, height)
										.add_image_usage_flags(VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT)
									.build();
		if (!swapchain_result) {
			EXPECT(false, "[VULKAN] {}", swapchain_result.error().message().c_str())
		}
		vkb::Swapchain& vkbswapchain = swapchain_result.value();
		_vkSwapchain = vkbswapchain.swapchain;
		_vkSwapchainImageViews = vkbswapchain.get_image_views().value();
		_vkSwapchainImages = vkbswapchain.get_images().value();
		_swapchainImageFormat = vkbswapchain.image_format;
		_vkSwapchianExtent = vkbswapchain.extent;
	}

	void VulkanEngine::CreateCommands() {
		// this needs to be altered later depending on the operations my engine needs
		VkCommandPoolCreateInfo poolInfo = vkinfo::CreateCommandPoolInfo(_vkGraphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		for (FrameData &_frame: _frames) {
			VK_CHECK(vkCreateCommandPool(_vkDevice, &poolInfo, nullptr, &_frame._commandPool));

			VkCommandBufferAllocateInfo allocInfo = vkinfo::CreateCommandBufferAllocateInfo(_frame._commandPool, 1);
			VK_CHECK(vkAllocateCommandBuffers(_vkDevice, &allocInfo, &_frame._commandBuffer));
		}

		// imm
		VK_CHECK(vkCreateCommandPool(_vkDevice, &poolInfo, nullptr, &_immCommandPool));
		VkCommandBufferAllocateInfo immCBAI = vkinfo::CreateCommandBufferAllocateInfo(_immCommandPool, 1);
		VK_CHECK(vkAllocateCommandBuffers(_vkDevice, &immCBAI, &_immCommandBuffer));
	}

	void VulkanEngine::CreateSyncStructures() {
		VkFenceCreateInfo fenceInfo = vkinfo::CreateFenceInfo(VK_FENCE_CREATE_SIGNALED_BIT);
		VkSemaphoreCreateInfo semaphoreInfo = vkinfo::CreateSemaphoreInfo();

		for (FrameData &_frame: _frames) {
			VK_CHECK(vkCreateFence(_vkDevice, &fenceInfo, nullptr, &_frame._renderFence));

			VK_CHECK(vkCreateSemaphore(_vkDevice, &semaphoreInfo, nullptr, &_frame._swapchainSemaphore));
			VK_CHECK(vkCreateSemaphore(_vkDevice, &semaphoreInfo, nullptr, &_frame._renderSemaphore));
		}
		// imm
		VK_CHECK(vkCreateFence(_vkDevice, &fenceInfo, nullptr, &_immFence));
	}
	void VulkanEngine::CreateDefaultData() {
		VkSamplerCreateInfo linearInfo = { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, .pNext = nullptr };
		linearInfo.magFilter = VK_FILTER_LINEAR;
		linearInfo.minFilter = VK_FILTER_LINEAR;
		linearInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		linearInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		linearInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		linearInfo.minLod = -1000;
		linearInfo.maxLod = 1000;
		linearInfo.maxAnisotropy = 1.0f;
		vkCreateSampler(_vkDevice, &linearInfo, nullptr, &default_LinearSampler);

		VkSamplerCreateInfo nearestInfo = { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, .pNext = nullptr };
		nearestInfo.magFilter = VK_FILTER_NEAREST;
		nearestInfo.minFilter = VK_FILTER_NEAREST;
		nearestInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		nearestInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		nearestInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		nearestInfo.minLod = -1000;
		nearestInfo.maxLod = 1000;
		nearestInfo.maxAnisotropy = 1.0f;
		vkCreateSampler(_vkDevice, &nearestInfo, nullptr, &default_NearestSampler);
	}
	void VulkanEngine::CreateFrameDescriptors() {
		// load up per frame descriptors
		for (auto &_frame: _frames) {
			// create a descriptor pool
			std::vector<DescriptorAllocatorGrowable::PoolSizeRatio> frame_sizes = {
					{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3},
					{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
					{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
					{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4}
			};
			_frame._frameDescriptors = DescriptorAllocatorGrowable {};
			_frame._frameDescriptors.Init(_vkDevice, 1000, frame_sizes);
		}
	}

	void VulkanEngine::CreateDefaultImages() {
		VkExtent2D tempextent = {
				_vkSwapchianExtent.width,
				_vkSwapchianExtent.height
		};
		// COLOR (final)
		{
			VkImageUsageFlags color_resolve_image_usages = {};
			color_resolve_image_usages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			color_resolve_image_usages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			color_resolve_image_usages |= VK_IMAGE_USAGE_SAMPLED_BIT;
			_colorResolveImage = this->CreateImage(tempextent, VK_FORMAT_R8G8B8A8_UNORM, color_resolve_image_usages, VK_SAMPLE_COUNT_1_BIT, false);
		}
		// MSAA COLOR (draw)
		{
			VkImageUsageFlags color_msaa_image_usages = {};
			color_msaa_image_usages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			color_msaa_image_usages |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
			_colorMSAAImage = this->CreateImage(tempextent, VK_FORMAT_R8G8B8A8_UNORM, color_msaa_image_usages, VK_SAMPLE_COUNT_4_BIT, false);
		}
		// MSAA DEPTH (draw)
		{
			VkImageUsageFlags depth_image_usages = {};
			depth_image_usages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			depth_image_usages |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
			_depthStencilMSAAImage = this->CreateImage(tempextent, VK_FORMAT_D32_SFLOAT_S8_UINT, depth_image_usages, VK_SAMPLE_COUNT_4_BIT, false);
		}
		// ENTITY (editor)
		{
			VkImageUsageFlags entity_image_usages = {};
			entity_image_usages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			entity_image_usages |= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			entity_image_usages |= VK_IMAGE_USAGE_STORAGE_BIT;
			_entityImage = this->CreateImage(tempextent, VK_FORMAT_R32_UINT, entity_image_usages, VK_SAMPLE_COUNT_1_BIT, false);
		}
		// MSAA ENTITY (editor)
		{
			VkImageUsageFlags entity_msaa_image_usages = {};
			entity_msaa_image_usages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			entity_msaa_image_usages |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
			_entityMSAAImage = this->CreateImage(tempextent, VK_FORMAT_R32_UINT, entity_msaa_image_usages, VK_SAMPLE_COUNT_4_BIT, false);
		}
		// VIEWPORT (ui)
		{
			VkImageUsageFlags viewport_image_usages = {};
			viewport_image_usages |= VK_IMAGE_USAGE_SAMPLED_BIT;
			viewport_image_usages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			_viewportImage = this->CreateImage(tempextent, VK_FORMAT_R8G8B8A8_UNORM, viewport_image_usages, VK_SAMPLE_COUNT_1_BIT, false);
		}
	}

	void VulkanEngine::DestroySwapchain() {
		for (VkImageView imageView : _vkSwapchainImageViews) {
			vkDestroyImageView(_vkDevice, imageView, nullptr);
		}
		// images are destroyed alongside swapchain destruction
		vkDestroySwapchainKHR(_vkDevice, _vkSwapchain, nullptr);
		_vkSwapchainImages.clear();
		_vkSwapchainImageViews.clear();
	}
	void VulkanEngine::OnResizeWindow(GLFWwindow* pWindow) {
		vkDeviceWaitIdle(_vkDevice);
		{
			// create swapchain
			this->DestroySwapchain();
			int w, h;
			glfwGetFramebufferSize(pWindow, &w, &h);
			this->CreateSwapchain(w, h);

			// recreate images
			this->DestroyImage(this->_colorMSAAImage);
			this->DestroyImage(this->_depthStencilMSAAImage);
			this->DestroyImage(this->_colorResolveImage);
			this->DestroyImage(this->_entityImage);
			this->DestroyImage(this->_viewportImage);
			this->CreateDefaultImages();

		}
		this->resizeRequested = false;
	}

	void VulkanEngine::DestroyEngine() {
		// wait before destruction process
		vkDeviceWaitIdle(_vkDevice);
		{
			/// destroy our pipelines
			{
//				this->DestroyPipeline(_wireframePipeline);
//				this->DestroyPipeline(_standardPipeline);
//				this->DestroyPipeline(_standardWireframePipeline);

			}
			// destroy default resources
			{
				vkDestroySampler(_vkDevice, default_NearestSampler, nullptr);
				vkDestroySampler(_vkDevice, default_LinearSampler, nullptr);
			}
			// destroy imgui resources
			{
				vkDestroyDescriptorPool(_vkDevice, _imguiDescriptorPool, nullptr);
			}
			// destroy vma images
			{
				this->DestroyImage(_colorResolveImage);
				this->DestroyImage(_colorMSAAImage);
				this->DestroyImage(_viewportImage);
				this->DestroyImage(_depthStencilMSAAImage);
				this->DestroyImage(_entityImage);
			}

			// destroy imm
			{
				vkDestroyFence(_vkDevice, _immFence, nullptr);
				vkFreeCommandBuffers(_vkDevice, _immCommandPool, 1, &_immCommandBuffer);
				vkDestroyCommandPool(_vkDevice, _immCommandPool, nullptr);
			}
			// destroy framedata (commands and sync structures)
			for (FrameData& _frame: _frames) {
				vkDestroyFence(_vkDevice, _frame._renderFence, nullptr);
				vkDestroySemaphore(_vkDevice, _frame._swapchainSemaphore, nullptr);
				vkDestroySemaphore(_vkDevice, _frame._renderSemaphore, nullptr);

				vkFreeCommandBuffers(_vkDevice, _frame._commandPool, 1, &_frame._commandBuffer);
				vkDestroyCommandPool(_vkDevice, _frame._commandPool, nullptr);

				_frame._frameDescriptors.DestroyPools(_vkDevice); // per frame descriptor pool destruction
			}
			// destroy swapchain & its resources
			// because we already have this functionality implemented for runtime swapchain recreation we just reuse it here
			{
				this->DestroySwapchain();
			}
			// destroy single objects
			{
				vmaDestroyAllocator(_allocator);// if you get validation errors regarding "device memory not destroyed" its this guy
				vkDestroySurfaceKHR(_vkInstance, _vkSurfaceKHR, nullptr);
				vkDestroyDevice(_vkDevice, nullptr);
				vkb::destroy_debug_utils_messenger(_vkInstance, _vkDebugMessenger);
				vkDestroyInstance(_vkInstance, nullptr);
			}
		}
	}

	void VulkanEngine::Aquire() {
		VK_CHECK(vkWaitForFences(this->_vkDevice, 1, &this->getCurrentFrameData()._renderFence, VK_TRUE, UINT64_MAX));
		this->getCurrentFrameData()._frameDescriptors.ClearPools(_vkDevice);

		VkResult aquireResult = vkAcquireNextImageKHR(
				this->_vkDevice,
				this->_vkSwapchain,
				UINT64_MAX,
				this->getCurrentFrameData()._swapchainSemaphore,
				nullptr,
				&this->_imageIndex);
		if (aquireResult == VK_ERROR_OUT_OF_DATE_KHR || aquireResult == VK_SUBOPTIMAL_KHR || resizeRequested) {
			this->resizeRequested = true;
			if (aquireResult == VK_ERROR_OUT_OF_DATE_KHR) return;
		}
		VK_CHECK(vkResetFences(this->_vkDevice, 1, &this->getCurrentFrameData()._renderFence));
		VK_CHECK(vkResetCommandPool(this->_vkDevice, this->getCurrentFrameData()._commandPool, 0));
	}

	void VulkanEngine::Present() {
		VkCommandBufferSubmitInfo cmdinfo = vkinfo::CreateCommandBufferSubmitInfo(this->getCurrentFrameData()._commandBuffer);
		VkSemaphoreSubmitInfo waitInfo = vkinfo::CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, this->getCurrentFrameData()._swapchainSemaphore);
		VkSemaphoreSubmitInfo signalInfo = vkinfo::CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, this->getCurrentFrameData()._renderSemaphore);

		VkSubmitInfo2 submitInfo = vkinfo::CreateSubmitInfo(&cmdinfo, &signalInfo, &waitInfo);
		vkQueueSubmit2KHR(this->_vkGraphicsQueue, 1, &submitInfo, this->getCurrentFrameData()._renderFence);


		VkPresentInfoKHR presentinfo = { .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, .pNext = nullptr };
		presentinfo.pSwapchains = &this->_vkSwapchain;
		presentinfo.swapchainCount = 1;
		presentinfo.pWaitSemaphores = &this->getCurrentFrameData()._renderSemaphore;
		presentinfo.waitSemaphoreCount = 1;
		presentinfo.pImageIndices = &this->_imageIndex;

		VkResult presentResult = vkQueuePresentKHR(this->_vkGraphicsQueue, &presentinfo);
		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || resizeRequested) {
			this->resizeRequested = true;
			if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) return;
		}
		this->_frameNum++;
	}

	// -- HELPER FUNCTIONS -- //

	void VulkanEngine::Immediate_Submit(std::function<void(VkCommandBuffer cmd)> &&function) {
		VK_CHECK(vkResetFences(_vkDevice, 1, &_immFence));
		VK_CHECK(vkResetCommandBuffer(_immCommandBuffer, 0));

		VkCommandBufferBeginInfo cmdBeginInfo = vkinfo::CreateCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		VK_CHECK(vkBeginCommandBuffer(_immCommandBuffer, &cmdBeginInfo));

		function(_immCommandBuffer);

		VK_CHECK(vkEndCommandBuffer(_immCommandBuffer));

		VkCommandBufferSubmitInfo cmdinfo = vkinfo::CreateCommandBufferSubmitInfo(_immCommandBuffer);
		VkSubmitInfo2 submit = vkinfo::CreateSubmitInfo(&cmdinfo, nullptr, nullptr);

		// submit command buffer to the queue and execute it.
		//  _renderFence will now block until the graphic commands finish execution
		VK_CHECK(vkQueueSubmit2KHR(_vkGraphicsQueue, 1, &submit, _immFence));

		VK_CHECK(vkWaitForFences(_vkDevice, 1, &_immFence, true, 9999999999));
	}

	vktypes::AllocatedBuffer VulkanEngine::CreateBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags) const {
		VkBufferCreateInfo bufferInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.pNext = nullptr;
		bufferInfo.size = allocSize;
		bufferInfo.usage = usage;

		VmaAllocationCreateInfo vmaallocInfo = {};
		vmaallocInfo.usage = memoryUsage;
		vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | flags;

		vktypes::AllocatedBuffer newBuffer = {};
		VK_CHECK(vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation, &newBuffer.info));
		return newBuffer;
	}

	vktypes::AllocatedImage VulkanEngine::CreateImage(const std::filesystem::path& file_path, VkFormat format, VkImageUsageFlags usage, bool mipmapped) {
		vktypes::AllocatedImage newImage = {};

		int width, height, nrChannels;
		void* data = stbi_load(Filesystem::ToDirectory(file_path).c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
		VkExtent3D extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
		if (data) {
			newImage = this->CreateImage(data, extent, format, usage, mipmapped);
		} else {
			fmt::print(stderr, "Failed to load image\n");
		}
		stbi_image_free(data);

		return newImage;
	}
	vktypes::AllocatedImage VulkanEngine::CreateImage(VkExtent2D extent, VkFormat format, VkImageUsageFlags usages, VkSampleCountFlags samples, bool mipmapped) const {
		VkExtent3D extent3D = {
			.width = extent.width,
			.height = extent.height,
			.depth = 1
		};
		return this->CreateImage(extent3D, format, usages, samples, mipmapped);
	}
	vktypes::AllocatedImage VulkanEngine::CreateImage(VkExtent3D extent, VkFormat format, VkImageUsageFlags usages, VkSampleCountFlags samples, bool mipmapped) const {
		vktypes::AllocatedImage newImage = {};
		newImage.imageFormat = format;
		newImage.imageExtent = extent;

		// small checking if mipmapping was true, than so some fancy math to get the levels we need
		uint32_t miplevels = 1;
		if (mipmapped) miplevels = static_cast<uint32_t>(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1;
		VkImageCreateInfo img_info = vkinfo::CreateImageInfo(extent, format, usages, miplevels, samples);

		VmaAllocationCreateInfo allocinfo = {};
		allocinfo.usage = VMA_MEMORY_USAGE_AUTO; // was gpu only mem but changed to auto
		allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK(vmaCreateImage(_allocator, &img_info, &allocinfo, &newImage.image, &newImage.allocation, &newImage.allocationInfo));

		VkImageViewCreateInfo view_info = vkinfo::CreateImageViewInfo(format, newImage.image, vkutil::AspectMaskFromFormat(format));
		view_info.subresourceRange.levelCount = img_info.mipLevels;
		VK_CHECK(vkCreateImageView(_vkDevice, &view_info, nullptr, &newImage.imageView));

		return newImage;
	}
	vktypes::AllocatedImage VulkanEngine::CreateImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped) {
		size_t data_size = size.depth * size.width * size.height * 4;
		vktypes::AllocatedBuffer uploadbuffer = this->CreateBuffer(data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		memcpy(uploadbuffer.info.pMappedData, data, data_size);

		vktypes::AllocatedImage new_image = this->CreateImage(size, format, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_SAMPLE_COUNT_1_BIT, mipmapped);

		this->Immediate_Submit([&](VkCommandBuffer cmd) {
			vkutil::TransitionImageLayout(cmd, new_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

			VkBufferImageCopy copyRegion = {};
			copyRegion.bufferOffset = 0;
			copyRegion.bufferRowLength = 0;
			copyRegion.bufferImageHeight = 0;

			copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageSubresource.baseArrayLayer = 0;
			copyRegion.imageSubresource.layerCount = 1;
			copyRegion.imageExtent = size;

			// copy the buffer into the image
			vkCmdCopyBufferToImage(cmd, uploadbuffer.buffer, new_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

			if (mipmapped) {
				vkutil::GenerateMipmaps(cmd, new_image.image, VkExtent2D{new_image.imageExtent.width,new_image.imageExtent.height});
			} else {
				vkutil::TransitionImageLayout(cmd, new_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
		});
		this->DestroyBuffer(uploadbuffer);
		return new_image;
	}

	vktypes::MeshData VulkanEngine::CreateMesh(std::vector<Vertex_Standard> vertices) {
		// just calculate sizes of our parameters
		const size_t vertexBufferSize = vertices.size() * sizeof(Vertex_Standard);

		vktypes::MeshData mesh = {};
		mesh.vertexCount = vertices.size();

		vktypes::StagingMeshBuffers surfaceBuffer = {};
		// create vertex buffer
		surfaceBuffer.vertexBuffer = this->CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		//find the adress of the vertex buffer
		VkBufferDeviceAddressInfo deviceAdressInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .pNext = nullptr };
		deviceAdressInfo.buffer = surfaceBuffer.vertexBuffer.buffer;
		mesh.constants.vertexBufferAddress = vkGetBufferDeviceAddress(_vkDevice, &deviceAdressInfo);


		vktypes::AllocatedBuffer staging = this->CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

		void *data = staging.allocation->GetMappedData();

		// copy vertex buffer
		memcpy(data, vertices.data(), vertexBufferSize);

		this->Immediate_Submit([&](VkCommandBuffer cmd) {
			VkBufferCopy vertexCopy{0};
			vertexCopy.dstOffset = 0;
			vertexCopy.srcOffset = 0;
			vertexCopy.size = vertexBufferSize;

			vkCmdCopyBuffer(cmd, staging.buffer, surfaceBuffer.vertexBuffer.buffer, 1, &vertexCopy);
		});

		this->DestroyBuffer(staging);
		return mesh;
	}

	vktypes::MeshData VulkanEngine::CreateMesh(std::vector<Vertex_Standard> vertices, std::vector<uint32_t> indices) {
		// just calculate sizes of our parameters
		const size_t vertexBufferSize = vertices.size() * sizeof(Vertex_Standard);
		const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

		vktypes::MeshData mesh = {};
		mesh.indexCount = indices.size();
		vktypes::StagingMeshBuffers surfaceBuffer = {};

		// create vertex buffer
		surfaceBuffer.vertexBuffer = this->CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		//find the adress of the vertex buffer
		VkBufferDeviceAddressInfo deviceAdressInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .pNext = nullptr};
		deviceAdressInfo.buffer = surfaceBuffer.vertexBuffer.buffer;
		mesh.constants.vertexBufferAddress = vkGetBufferDeviceAddress(_vkDevice, &deviceAdressInfo);

		//create index buffer
		mesh.indexBuffer = this->CreateBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		vktypes::AllocatedBuffer staging = this->CreateBuffer(vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

		void *data = staging.allocation->GetMappedData();

		// copy vertex buffer
		memcpy(data, vertices.data(), vertexBufferSize);
		// copy index buffer
		memcpy((char *) data + vertexBufferSize, indices.data(), indexBufferSize);

		this->Immediate_Submit([&](VkCommandBuffer cmd) {
			VkBufferCopy vertexCopy{0};
			vertexCopy.dstOffset = 0;
			vertexCopy.srcOffset = 0;
			vertexCopy.size = vertexBufferSize;

			vkCmdCopyBuffer(cmd, staging.buffer, surfaceBuffer.vertexBuffer.buffer, 1, &vertexCopy);

			VkBufferCopy indexCopy{0};
			indexCopy.dstOffset = 0;
			indexCopy.srcOffset = vertexBufferSize;
			indexCopy.size = indexBufferSize;

			vkCmdCopyBuffer(cmd, staging.buffer, mesh.indexBuffer.buffer, 1, &indexCopy);
		});

		this->DestroyBuffer(staging);
		return mesh;
	}


	void VulkanEngine::DestroyBuffer(const vktypes::AllocatedBuffer& allocatedBuffer) const {
		vmaDestroyBuffer(_allocator, allocatedBuffer.buffer, allocatedBuffer.allocation);
	}
	void VulkanEngine::DestroyImage(const vktypes::AllocatedImage& allocatedImage) const {
		vkDestroyImageView(_vkDevice, allocatedImage.imageView, nullptr);
		vmaDestroyImage(_allocator, allocatedImage.image, allocatedImage.allocation);
	}

	void VulkanEngine::DrawMeshData(const vktypes::MeshData& data) {
		if (data.IsIndexed()) {
			vkCmdBindIndexBuffer(this->getCurrentFrameData()._commandBuffer, data.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(this->getCurrentFrameData()._commandBuffer, data.indexCount, 1, 0, 0, 0);
		} else {
			vkCmdDraw(this->getCurrentFrameData()._commandBuffer, data.vertexCount, 1, 0, 0);
		}
	}
	void VulkanEngine::SetupImGui() {
		// imgui requires pool
		//  the size of the pool is very oversize, but it's copied from imgui demo
		VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
											 {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
											 {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
											 {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
											 {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
											 {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
											 {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
											 {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
											 {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
											 {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
											 {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
		pool_info.pPoolSizes = pool_sizes;

		VK_CHECK(vkCreateDescriptorPool(this->_vkDevice, &pool_info, nullptr, &this->_imguiDescriptorPool));
	}
}


